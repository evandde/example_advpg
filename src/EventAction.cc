#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "g4csv.hh"

#include "EventAction.hh"

EventAction::EventAction()
    : G4UserEventAction(), fHCID(-1)
{
}

EventAction::~EventAction()
{
}

void EventAction::BeginOfEventAction(const G4Event *)
{
}

void EventAction::EndOfEventAction(const G4Event *anEvent)
{
    auto HCE = anEvent->GetHCofThisEvent();
    if (!HCE)
        return;

    if (fHCID == -1)
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID("Detector/EDep");

    auto hitsMap = static_cast<G4THitsMap<G4double> *>(HCE->GetHC(fHCID));

    auto analysisManager = G4AnalysisManager::Instance();

    for (const auto &iter : *(hitsMap->GetMap()))
    {
        auto eDep = *(iter.second);
        if (eDep > 0.)
        {
            auto weight = anEvent->GetPrimaryVertex()->GetWeight();
            analysisManager->FillH1(0, eDep / weight, weight);

            analysisManager->FillNtupleIColumn(0, anEvent->GetEventID());
            analysisManager->FillNtupleDColumn(1, eDep / weight);
            analysisManager->FillNtupleDColumn(2, weight);
            analysisManager->AddNtupleRow();
        }
    }
}
