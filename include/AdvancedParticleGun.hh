/// \author Evan Kim
/// \email evandde@gmail.com
/// \homepage evandde.github.io

#ifndef ADVANCEDPARTICLEGUN_HH
#define ADVANCEDPARTICLEGUN_HH

#include "G4ParticleGun.hh"
#include "G4PhysicalVolumeStore.hh"

class AdvancedParticleGun : public G4ParticleGun
{
public:
    AdvancedParticleGun();
    ~AdvancedParticleGun();

    virtual void GeneratePrimaryVertex(G4Event *);

    inline void SetSourceVolume(G4VPhysicalVolume *sourceVol)
    {
        if (!sourceVol)
            G4cout << "WARNING: Invalid source PV\n\n";
        fSourceVol = sourceVol;
    }
    inline void SetSourceVolume(G4String sourceVolName)
    {
        auto sourceVol = G4PhysicalVolumeStore::GetInstance()->GetVolume(sourceVolName);
        if (!sourceVol)
            G4cout << "WARNING: Invalid source PV\n\n";
        fSourceVol = sourceVol;
    }
    inline G4VPhysicalVolume *GetSourceVolume() const { return fSourceVol; }

    inline void SetTargetVolume(G4VPhysicalVolume *targetVol, G4double margin = 0.)
    {
        if (!targetVol)
            G4cout << "WARNING: Invalid target PV\n\n";
        fTargetVol = targetVol;
        fTargetVolumeMargin = margin;
    }
    inline void SetTargetVolume(G4String targetVolName, G4double margin = 0.)
    {
        auto targetVol = G4PhysicalVolumeStore::GetInstance()->GetVolume(targetVolName);
        if (!targetVol)
            G4cout << "WARNING: Invalid target PV\n\n";
        fTargetVol = targetVol;
        fTargetVolumeMargin = margin;
    }
    inline void SetTargetVolumeMargin(G4double margin) { fTargetVolumeMargin = margin; }
    inline G4VPhysicalVolume *GetTargetVolume() const { return fTargetVol; }
    inline G4double GetTargetVolumeMargin() const { return fTargetVolumeMargin; }
    inline void SetNuclideSource(G4String nuclideName) { fNuclideName = nuclideName; }
    inline G4String GetNuclideSource() const { return fNuclideName; }
    inline void SetMinPhotonEnergy(G4double minPhotonEnergy) { fMinPhotonEnergy = minPhotonEnergy; }
    inline G4double GetMinPhotonEnergy() const { return fMinPhotonEnergy; }

protected:
    G4VPhysicalVolume *fSourceVol;
    G4VPhysicalVolume *fTargetVol;
    G4double fTargetVolumeMargin;
    G4String fNuclideName;
    G4double fMinPhotonEnergy;
    G4ThreeVector ConvertCoordVolume2World(const G4VPhysicalVolume *const pv, const G4ThreeVector pt = G4ThreeVector());
    G4ThreeVector SamplePointFromVolume(const G4VPhysicalVolume *const pv);
    G4double GetApexHalfAngleToVolume(const G4ThreeVector pt, const G4VPhysicalVolume *const pv, const G4double margin = 0.);
};

#endif
