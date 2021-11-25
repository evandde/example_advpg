# AdvancedParticleGun
Advanced Particle Gun class for Geant4.



## Purpose

- It helps to set a G4PVPlacement type physical volume to be a primary source.
- It helps to set a G4PVPlacement type physical volume to be a target of a primary source.
- It helps to set photons from decay of a nuclide to be a primary source based on ICRP publication 107.



## What You Need

ICRP07DATA/

ICRP07DATA/ICRP-07.NDX

ICRP07DATA/ICRP-07.RAD

include/AdvancedParticleGun.hh

include/ICRP07Manager.hh

src/AdvancedParticleGun.cc

src/ICRP07Manager.cc



## Features

- The AdvancedParticleGun class is derived from G4ParticleGun; you can use member functions of G4ParticleGun class without a hitch. (e.g. SetParticlePosition(), SetParticleDefinition(), ...)
- AdvancedParticleGun::SetSourceVolume(G4String sourceVolName) function sets a G4PVPlacement object named *sourceVolName* to be a primary source term.
  - Primary particle positions will be sampled uniformly inside the volume. (using rejection method)
  - The *sourceVolName* must be unique.
- AdvancedParticleGun::SetTargetVolume(G4String targetVolName, G4double margin = 0.) function sets a G4PVPlacement object named *targetVolName* to be a target.
  - Primary particle directions will be sampled uniformly & isotropically within conical solid angle that completely surrounds the target volume (+ margin).
  - The *targetVolName* must be unique.
  - The particle weight (biasing) will be multiplied by *solid angle / 4pi*.
- AdvancedParticleGun::SetNuclideSource(G4String nuclideName) function sets a nuclide to be a gamma-ray primary source term.
  - The gamma-rays from the nuclide will be set to be primary particles, corresponding to the yield and decay chain (fractions of daughter nuclides) described in ICRP107.
  - *nuclideName* must be written in the following form: "Cs-137", "Co-60", ...
  - The function only considers photons (X-rays, gamma-rays, annihilation photons).
  - Users can set minimum energy of primary photons by using AdvancedParticleGun::SetMinPhotonEnergy(G4double minPhotonEnergy) in order to ignore production of low energy X-rays (e.g. a few keV X-rays).
  - The particle weight (biasing) will be multiplied by *total yield*.
  - The function does NOT consider half-lives of daughter nuclides, so that the actual activities of the daughter nuclides in real case might be different.


## How To Use

1. Copy files below to your project:
   - ICRP07DATA/
     - ICRP07DATA/ICRP-07.NDX
     - ICRP07DATA/ICRP-07.RAD
   - include/AdvancedParticleGun.hh
   - include/ICRP07Manager.hh
   - src/AdvancedParticleGun.cc
   - src/ICRP07Manager.cc
2. In your own class derived from G4VUserPrimaryGeneratorAction class, replace G4ParticleGun* type class member to AdvancedParticleGun* type one.
3. That's all! Have fun.


## License
The files in `ICRP07DATA` directory is from ICRP 107 copyright 2008 Eckerman and Endo. This data is provided with the following text in the license.txt file:

ICRP-07 Data Files Copyright Notice

Copyright (C) 2008 A. Endo and K.F. Eckerman, Authors. All Rights Reserved.

Permission is hereby granted to any person obtaining a copies of the ICRP-07 data files and associated documentation to use, copy, and distribute these materials and the documentation for educational, research, and not-for-profit purposes, without fee and without a signed licensing agreement provided that the file LICENSE.TXT containing the above copyright notice, this paragraph and the following two paragraphs appears in all copies, modifications, and distributions.

IN NO EVENT SHALL THE AUTHORS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THE SOFTWARE, ITS DATA FILES, AND THE DOCUMENTATION.

THE AUTHORS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE, DATA FILES AND ACCOMPANYING DOCUMENTATION PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHORS HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, FUTURE UPDATES, ENHANCEMENTS, OR MODIFICATIONS TO THE SOFTWARE.
