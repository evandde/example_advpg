/// \author Evan Kim
/// \email evandde@gmail.com
/// \homepage evandde.github.io

#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "ICRP07Manager.hh"

#include <fstream>

ICRP07Manager *ICRP07Manager::instance = nullptr;
std::map<G4String, DecayData> ICRP07Manager::fDecayDatabase;
std::map<G4String, RadiationData> ICRP07Manager::fRadiationDatabase;
#ifdef G4MULTITHREADED
G4Mutex ICRP07Manager::ICRP07ManagerMutex = G4MUTEX_INITIALIZER;
#endif

ICRP07Manager *ICRP07Manager::Instance()
{
    if (instance == nullptr)
    {
#ifdef G4MULTITHREADED
        G4MUTEXLOCK(&ICRP07ManagerMutex);
        if (instance == nullptr)
        {
#endif
            static ICRP07Manager manager;
            instance = &manager;
#ifdef G4MULTITHREADED
        }
        G4MUTEXUNLOCK(&ICRP07ManagerMutex);
#endif
    }

    return instance;
}

ICRP07Manager::~ICRP07Manager()
{
    fDecayDatabase.clear();
    fRadiationDatabase.clear();
}

ICRP07Manager::ICRP07Manager()
{
    if (fDecayDatabase.empty())
        ImportNDX("../ICRP07DATA/ICRP-07.NDX");

    if (fRadiationDatabase.empty())
        ImportRAD("../ICRP07DATA/ICRP-07.RAD");
}

RadiationData ICRP07Manager::GetPhotonSource(G4String nuclideName) const
{
    RadiationData photonSource;

    if (fDecayDatabase.find(nuclideName) == fDecayDatabase.end())
        return photonSource;
    if (fRadiationDatabase.find(nuclideName) == fRadiationDatabase.end())
        return photonSource;

    auto radiationData = fRadiationDatabase.at(nuclideName);
    AppendRadiationData(photonSource, radiationData);

    return photonSource;
}

RadiationData ICRP07Manager::GetPhotonSourceAllDaughters(G4String nuclideName) const
{
    RadiationData photonSource;
    if (fDecayDatabase.find(nuclideName) == fDecayDatabase.end())
        return photonSource;
    if (fRadiationDatabase.find(nuclideName) == fRadiationDatabase.end())
        return photonSource;

    auto radiationData = fRadiationDatabase.at(nuclideName);
    AppendRadiationData(photonSource, radiationData);

    auto decayData = fDecayDatabase.at(nuclideName);
    for (size_t i = 0; i < decayData.fDaughterNuclideNames.size(); ++i)
    {
        auto daughterNuclideName = decayData.fDaughterNuclideNames[i];
        auto daughterNuclideBranchRatio = decayData.fDaughterNuclideRatios[i];

        auto daughterPhotonSource = GetPhotonSourceAllDaughters(daughterNuclideName);
        AppendRadiationData(photonSource, daughterPhotonSource, daughterNuclideBranchRatio);
    }

    return photonSource;
}

void ICRP07Manager::AppendRadiationData(RadiationData &originalData, RadiationData newData, G4double yieldMultiplier) const
{
    std::for_each(newData.fYields.begin(), newData.fYields.end(),
                  [yieldMultiplier](G4double &c)
                  { c *= yieldMultiplier; });

    originalData.fPhotonEnergies.insert(originalData.fPhotonEnergies.end(), newData.fPhotonEnergies.begin(), newData.fPhotonEnergies.end());
    originalData.fYields.insert(originalData.fYields.end(), newData.fYields.begin(), newData.fYields.end());
}

void ICRP07Manager::RemoveRadiationDataByMinimumEnergy(RadiationData &originalData, G4double minimumEnergy) const
{
    auto iterEnergies = originalData.fPhotonEnergies.begin();
    auto iterYields = originalData.fYields.begin();

    while (iterEnergies != originalData.fPhotonEnergies.end())
    {
        if (*iterEnergies < minimumEnergy)
        {
            iterEnergies = originalData.fPhotonEnergies.erase(iterEnergies);
            iterYields = originalData.fYields.erase(iterYields);
        }
        else
        {
            ++iterEnergies;
            ++iterYields;
        }
    }
}

void ICRP07Manager::ImportNDX(G4String filepath)
{
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ios::in);
    if (!ifs.is_open())
    {
        G4cerr << "WARNING: There is no " << filepath << ".\n";
        return;
    }

    G4String dummy, tmp, theLine;

    std::getline(ifs, dummy); // skip header

    while (!ifs.eof())
    {
        G4String name;
        DecayData DecayData;

        std::getline(ifs, theLine);
        std::stringstream ss(theLine);

        ss >> name >> DecayData.fHalfLife >> DecayData.fDecayMode;
        for (size_t i = 0; i < 4; ++i)
            ss >> dummy;
        for (size_t i = 0; i < 4; ++i)
        {
            ss >> tmp;
            if (tmp == "0")
                break;
            else
            {
                DecayData.fDaughterNuclideNames.push_back(tmp);
                ss >> dummy >> tmp;
                DecayData.fDaughterNuclideRatios.push_back(G4UIcommand::ConvertToDouble(tmp));
            }
        }

        fDecayDatabase[name] = DecayData;
    }

    ifs.close();
}

void ICRP07Manager::ImportRAD(G4String filepath)
{
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ios::in);
    if (!ifs.is_open())
    {
        G4cerr << "WARNING: There is no " << filepath << ".\n";
        return;
    }

    G4String dummy, tmp, theLine;
    G4String name, nextName;

    while (!ifs.eof())
    {
        G4int iCode;
        RadiationData RadiationData;

        while (!ifs.eof())
        {
            std::getline(ifs, theLine);

            if (theLine.c_str()[0] >= 'A' && theLine.c_str()[0] <= 'Z')
            {
                std::stringstream ss(theLine);
                ss >> nextName;
                break;
            }

            std::stringstream ss(theLine);
            ss >> iCode;
            if (iCode == 1 || iCode == 2 || iCode == 3) // save only Gamma, X, Annihilation
            {
                ss >> tmp;
                RadiationData.fYields.push_back(G4UIcommand::ConvertToDouble(tmp));
                ss >> tmp;
                RadiationData.fPhotonEnergies.push_back(G4UIcommand::ConvertToDouble(tmp) * MeV);
            }
        }

        if (!name.empty())
            fRadiationDatabase[name] = RadiationData;
        name = nextName;
    }

    ifs.close();
}

void ICRP07Manager::PrintNDX() const
{
    for (const auto &nuclide : fDecayDatabase)
    {
        G4cout << nuclide.first << " "
               << nuclide.second.fHalfLife << " "
               << nuclide.second.fDecayMode << " ";
        for (size_t i = 0; i < nuclide.second.fDaughterNuclideNames.size(); ++i)
        {
            G4cout << nuclide.second.fDaughterNuclideNames[i] << " "
                   << nuclide.second.fDaughterNuclideRatios[i] << " ";
        }
        G4cout << G4endl;
    }
}

void ICRP07Manager::PrintRAD() const
{
    for (const auto &radiationData : fRadiationDatabase)
    {
        G4cout << radiationData.first << G4endl;
        PrintRadiationDataBrief(radiationData.second);
    }
}

void ICRP07Manager::PrintRADofNuclide(G4String nuclideName) const
{
    if (fRadiationDatabase.find(nuclideName) == fRadiationDatabase.end())
        return;

    auto radiationData = fRadiationDatabase.at(nuclideName);

    G4cout << nuclideName << G4endl;
    PrintRadiationData(radiationData);
}

void ICRP07Manager::PrintRadiationDataBrief(const RadiationData &radiationData) const
{
    if (radiationData.fYields.size() > 3)
    {
        for (size_t i = 0; i < 3; ++i)
        {
            G4cout << " -- "
                   << radiationData.fYields[i] << " "
                   << radiationData.fPhotonEnergies[i] << " "
                   << G4endl;
        }
        G4cout << " -- ...\n";
        G4cout << " -- "
               << radiationData.fYields[radiationData.fYields.size() - 1] << " "
               << radiationData.fPhotonEnergies[radiationData.fYields.size() - 1] << " "
               << G4endl;
    }
    else
    {
        for (size_t i = 0; i < radiationData.fYields.size(); ++i)
        {
            G4cout << " -- "
                   << radiationData.fYields[i] << " "
                   << radiationData.fPhotonEnergies[i] << " "
                   << G4endl;
        }
    }
}

void ICRP07Manager::PrintRadiationData(const RadiationData &radiationData) const
{
    for (size_t i = 0; i < radiationData.fYields.size(); ++i)
    {
        G4cout << " -- "
               << radiationData.fYields[i] << " "
               << radiationData.fPhotonEnergies[i] << " "
               << G4endl;
    }
}