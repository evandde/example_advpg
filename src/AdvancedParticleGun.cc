/// \author Evan Kim
/// \email evandde@gmail.com
/// \homepage evandde.github.io

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "G4RandomTools.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4TransportationManager.hh"
#include "G4UIcommand.hh"
#include "G4Gamma.hh"

#include "AdvancedParticleGun.hh"
#include "ICRP07Manager.hh"

AdvancedParticleGun::AdvancedParticleGun()
    : fSourceVol(nullptr), fTargetVol(nullptr), fTargetVolumeMargin(0.), fNuclideName(std::string()), fMinPhotonEnergy(0.), G4ParticleGun()
{
}

AdvancedParticleGun::~AdvancedParticleGun()
{
}

void AdvancedParticleGun::GeneratePrimaryVertex(G4Event *event)
{
    G4double particleWeight = 1.;

    if (fSourceVol)
    {
        auto srcPosInVolume = SamplePointFromVolume(fSourceVol);
        auto srcPos = ConvertCoordVolume2World(fSourceVol, srcPosInVolume);
        SetParticlePosition(srcPos);
    }

    if (fTargetVol)
    {
        auto srcPos = GetParticlePosition();
        auto apexHalfAngle = GetApexHalfAngleToVolume(srcPos, fTargetVol, fTargetVolumeMargin);
        auto cosApexHalfAngle = std::cos(apexHalfAngle);

        G4ThreeVector dir;
        if (cosApexHalfAngle <= 0.)
            dir = G4RandomDirection();
        else
        {
            particleWeight *= (1 - cosApexHalfAngle) / 2.;

            dir = G4RandomDirection(cosApexHalfAngle);
            auto dirToVolume = ConvertCoordVolume2World(fTargetVol) - srcPos;
            auto zUnit = G4ThreeVector(0., 0., 1.);
            if (!(dirToVolume.isParallel(zUnit)))
                dir.rotate(G4ThreeVector(0., 0., 1.).cross(dirToVolume), G4ThreeVector(0., 0., 1.).angle(dirToVolume));
            else
                dir *= dirToVolume.unit().dot(zUnit);
        }
        SetParticleMomentumDirection(dir);
    }

    if (!fNuclideName.empty())
    {
        auto icrp107 = ICRP07Manager::Instance();
        auto photonSource = icrp107->GetPhotonSourceAllDaughters(fNuclideName);
        icrp107->RemoveRadiationDataByMinimumEnergy(photonSource, fMinPhotonEnergy);

        auto totalYield = 0.;
        for (const auto &yield : photonSource.fYields)
            totalYield += yield;
        particleWeight *= totalYield;

        auto sampler = G4RandGeneral(photonSource.fYields.data(), static_cast<G4int>(photonSource.fYields.size()), 1);
        auto idx = static_cast<G4int>(std::round(sampler.shoot() * photonSource.fYields.size()));
        auto energy = photonSource.fPhotonEnergies[idx];
        SetParticleEnergy(energy);

        SetParticleDefinition(G4Gamma::Definition());
    }

    G4ParticleGun::GeneratePrimaryVertex(event);
    event->GetPrimaryVertex()->SetWeight(particleWeight);
}

G4ThreeVector AdvancedParticleGun::SamplePointFromVolume(const G4VPhysicalVolume *const pv)
{
    auto sol = pv->GetLogicalVolume()->GetSolid();
    G4ThreeVector boundMin, boundMax;
    sol->BoundingLimits(boundMin, boundMax);
    G4ThreeVector pt;
    while (true)
    {
        pt = G4ThreeVector(G4RandFlat::shoot(boundMin.x(), boundMax.x()),
                           G4RandFlat::shoot(boundMin.y(), boundMax.y()),
                           G4RandFlat::shoot(boundMin.z(), boundMax.z()));
        if (kInside == sol->Inside(pt))
            break;
    }
    return pt;
}

G4double AdvancedParticleGun::GetApexHalfAngleToVolume(const G4ThreeVector ptInWorld, const G4VPhysicalVolume *const pv, const G4double margin)
{
    auto sol = pv->GetLogicalVolume()->GetSolid();
    G4ThreeVector boundMin, boundMax;
    sol->BoundingLimits(boundMin, boundMax);
    boundMin -= G4ThreeVector(margin, margin, margin);
    boundMax += G4ThreeVector(margin, margin, margin);

    auto boundMinInWorld = ConvertCoordVolume2World(pv, boundMin);
    auto boundMaxInWorld = ConvertCoordVolume2World(pv, boundMax);
    auto pvCenterInWorld = ConvertCoordVolume2World(pv);

    auto directionToVolumeCenter = pvCenterInWorld - ptInWorld;
    G4ThreeVectorList directionToVolumeCorners = {
        G4ThreeVector(boundMinInWorld.x(), boundMinInWorld.y(), boundMinInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMinInWorld.x(), boundMinInWorld.y(), boundMaxInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMinInWorld.x(), boundMaxInWorld.y(), boundMinInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMinInWorld.x(), boundMaxInWorld.y(), boundMaxInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMaxInWorld.x(), boundMinInWorld.y(), boundMinInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMaxInWorld.x(), boundMinInWorld.y(), boundMaxInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMaxInWorld.x(), boundMaxInWorld.y(), boundMinInWorld.z()) - ptInWorld,
        G4ThreeVector(boundMaxInWorld.x(), boundMaxInWorld.y(), boundMaxInWorld.z()) - ptInWorld};

    auto apexHalfAngle = 0.;
    for (const auto &directionToVolumeCorner : directionToVolumeCorners)
    {
        auto angle = directionToVolumeCenter.angle(directionToVolumeCorner);
        apexHalfAngle = (angle > apexHalfAngle) ? angle : apexHalfAngle;
    }

    return apexHalfAngle;
}

G4ThreeVector AdvancedParticleGun::ConvertCoordVolume2World(const G4VPhysicalVolume *const pv, const G4ThreeVector pt)
{
    auto ptInWorldCoord = pt;
    auto currentPV = pv;
    while (auto lvM = currentPV->GetMotherLogical())
    {
        ptInWorldCoord *= currentPV->GetObjectRotationValue();
        ptInWorldCoord += currentPV->GetObjectTranslation();

        currentPV = G4PhysicalVolumeStore::GetInstance()->GetVolume(lvM->GetName());
    }

    return ptInWorldCoord;
}