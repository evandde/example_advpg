/// \author Evan Kim
/// \email evandde@gmail.com
/// \homepage evandde.github.io

#ifndef ICRP07MANAGER_HH
#define ICRP07MANAGER_HH

#include "G4Threading.hh"

struct DecayData
{
    G4String fHalfLife;
    G4String fDecayMode;
    std::vector<G4String> fDaughterNuclideNames;
    std::vector<G4double> fDaughterNuclideRatios;
};

struct RadiationData
{
    std::vector<G4double> fPhotonEnergies;
    std::vector<G4double> fYields;
};

class ICRP07Manager
{
public:
    static ICRP07Manager *Instance();
    ~ICRP07Manager();

    RadiationData GetPhotonSource(G4String nuclideName) const;
    RadiationData GetPhotonSourceAllDaughters(G4String nuclideName) const;
    
    void RemoveRadiationDataByMinimumEnergy(RadiationData &originalData, G4double minimumEnergy) const;

    void PrintNDX() const;
    void PrintRAD() const;
    void PrintRADofNuclide(G4String nuclideName) const;
    void PrintRadiationDataBrief(const RadiationData& radiationData) const;
    void PrintRadiationData(const RadiationData& radiationData) const;

private:
    explicit ICRP07Manager();

    static ICRP07Manager *instance;
    static std::map<G4String, DecayData> fDecayDatabase;
    static std::map<G4String, RadiationData> fRadiationDatabase;

    void ImportNDX(G4String filepath);
    void ImportRAD(G4String filepath);

    void AppendRadiationData(RadiationData &originalData, RadiationData newData, G4double yieldMultiplier = 1.) const;

#ifdef G4MULTITHREADED
    static G4Mutex ICRP07ManagerMutex;
#endif
};

#endif