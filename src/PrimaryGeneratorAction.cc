#include "G4Gamma.hh"
#include "G4RandomTools.hh"
#include "G4SystemOfUnits.hh"

#include "PrimaryGeneratorAction.hh"
#include "AdvancedParticleGun.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    fPrimary = new AdvancedParticleGun();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fPrimary;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent)
{
    fPrimary->SetNuclideSource("Cs-137");
    fPrimary->SetMinPhotonEnergy(10. * keV);
    fPrimary->SetSourceVolume("Source");
    fPrimary->SetTargetVolume("Detector", 5. * cm);

    fPrimary->GeneratePrimaryVertex(anEvent);
}
