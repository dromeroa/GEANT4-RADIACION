//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4Material.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
 
  std::vector<G4String> elements = {"C", "H", "O"};
  std::vector<G4int> natoms      = {3, 4, 2};

  G4double density = 1.24 * g/cm3; // Ajustar según el infill si es impreso en 3D

  G4Material* PLA = nist->ConstructNewMaterial("PLA", elements, natoms, density);

  // Imprimir propiedades del PLA directamente en la terminal al iniciar:
  G4cout << "\n==========================================" << G4endl;
  G4cout << PLA << G4endl;
  G4cout << "==========================================\n" << G4endl;



  // Envelope parameters
  //
  G4double env_sizeXY = 20 * cm, env_sizeZ = 30 * cm;
  G4Material* env_mat = nist->FindOrBuildMaterial("G4_AIR");

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // World
  //
  G4double world_sizeXY = 1.2 * env_sizeXY;
  G4double world_sizeZ = 1.2 * env_sizeZ;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  auto solidWorld =
    new G4Box("World",  // its name
              0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);  // its size

  auto logicWorld = new G4LogicalVolume(solidWorld,  // its solid
                                        world_mat,  // its material
                                        "World");  // its name

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
                                     G4ThreeVector(),  // at (0,0,0)
                                     logicWorld,  // its logical volume
                                     "World",  // its name
                                     nullptr,  // its mother  volume
                                     false,  // no boolean operation
                                     0,  // copy number
                                     checkOverlaps);  // overlaps checking

  //
  // Envelope
  //
  auto solidEnv = new G4Box("Envelope",  // its name
                            0.5 * env_sizeXY, 0.5 * env_sizeXY, 0.5 * env_sizeZ);  // its size

  auto logicEnv = new G4LogicalVolume(solidEnv,  // its solid
                                      env_mat,  // its material
                                      "Envelope");  // its name


  new G4PVPlacement(nullptr,  // no rotation
                    G4ThreeVector(),  // at (0,0,0)
                    logicEnv,  // its logical volume
                    "Envelope",  // its name
                    logicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking



  //
  // Shape 1
//----------------------------- NUEVO ------------------------------------------
// 1. Obtener el material de polietileno
//G4Material* poly_mat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");

// 2. Crear la forma sólida (5 cm hacia cada lado = 10 cm total)
G4Box* solidBlock = new G4Box("PolyBlock", 5*cm, 5*cm, 5*cm);


// 3. Crear el volumen lógico (asignar el material a la forma) (PLA):
G4LogicalVolume* logicBlock =
  new G4LogicalVolume(solidBlock, PLA, "LogicBlock");

// 4. Posicionar el volumen físico en el centro del Envelope (Env)
new G4PVPlacement(0,                       // sin rotación
                  G4ThreeVector(0, 0, 0),  // en el centro (0,0,0)
                  logicBlock,              // el volumen lógico
                  "PhysBlock",             // su nombre
                  logicEnv,                // el volumen madre (Envelope)
                  false,                   // operaciones booleanas
                  0,                       // número de copia
                  checkOverlaps);          // verificar superposiciones
//-------------------------------------------------------------------------
/*
//
  // Cambiamos el material por polyetileno
//  G4NistManager* nist = G4NistManager::Instance();

G4Material* shape1_mat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  //G4Material* shape1_mat = nist->FindOrBuildMaterial("G4_A-150_TISSUE");
  G4ThreeVector pos1 = G4ThreeVector(0, 2 * cm, -7 * cm);

  // Conical section shape
  G4double shape1_rmina = 0. * cm, shape1_rmaxa = 2. * cm;
  G4double shape1_rminb = 0. * cm, shape1_rmaxb = 4. * cm;
  G4double shape1_hz = 3. * cm;
  G4double shape1_phimin = 0. * deg, shape1_phimax = 360. * deg;
  auto solidShape1 = new G4Cons("Shape1", shape1_rmina, shape1_rmaxa, shape1_rminb, shape1_rmaxb,
                                shape1_hz, shape1_phimin, shape1_phimax);

  auto logicShape1 = new G4LogicalVolume(solidShape1,  // its solid
                                         shape1_mat,  // its material
                                         "Shape1");  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    pos1,  // at position
                    logicShape1,  // its logical volume
                    "Shape1",  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking

  //
  // Shape 2
  //
  G4Material* shape2_mat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
  G4ThreeVector pos2 = G4ThreeVector(0, -1 * cm, 7 * cm);

  // Trapezoid shape
  G4double shape2_dxa = 12 * cm, shape2_dxb = 12 * cm;
  G4double shape2_dya = 10 * cm, shape2_dyb = 16 * cm;
  G4double shape2_dz = 6 * cm;
  auto solidShape2 =
    new G4Trd("Shape2",  // its name
              0.5 * shape2_dxa, 0.5 * shape2_dxb, 0.5 * shape2_dya, 0.5 * shape2_dyb,
              0.5 * shape2_dz);  // its size

  auto logicShape2 = new G4LogicalVolume(solidShape2,  // its solid
                                         shape2_mat,  // its material
                                         "Shape2");  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    pos2,  // at position
                    logicShape2,  // its logical volume
                    "Shape2",  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking

*/

  // Set Shape2 as scoring volume
  //
  //fScoringVolume = logicShape2;
  fScoringVolume = logicBlock;
  //
  // always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
