#include "DetectorConstruction.hpp"
#include "DetectorMessenger.hpp"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction(), fDetectorMessenger(0) {
	fDetectorMessenger = new DetectorMessenger(this);

	  fKaptonThickness = .2*mm; // can be overwritten by /MM/setKaptonThickness
	 fCoatingThickness = .1*mm; // can be overwritten by /MM/setCoatingThickness
	fDetectorThickness = 2.*mm; // can be overwritten by /MM/setDetectorThickness
}

DetectorConstruction::~DetectorConstruction() {
	delete fDetectorMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {  
	G4NistManager* nist = G4NistManager::Instance();

	G4bool checkOverlaps = true;

	//[[[cog from MMconfig import *; cog.outl("G4double z_kathode = {}*cm;".format(conf["photoconversion"]["z_kathode"])) ]]]
	G4double z_kathode = 1.*cm;
	//[[[end]]]

	// World
	//[[[cog from MMconfig import *; cog.outl("G4double sizeX_world = {}*cm, sizeY_world = {}*cm;".format(conf["detector"]["size_x"], conf["detector"]["size_y"])) ]]]
	G4double sizeX_world = 10.*cm, sizeY_world = 10.*cm;
	//[[[end]]]
	G4double sizeZ_world  = 2.*(fKaptonThickness + fCoatingThickness + z_kathode + 1*cm); // 1cm space above the detector
	G4Material* mat_air = nist->FindOrBuildMaterial("G4_AIR");

	G4Box* solid_world = new G4Box("World", .5*sizeX_world, .5*sizeY_world, .5*sizeZ_world);
	fLogicWorld = new G4LogicalVolume(solid_world, mat_air, "World");
	fPhysWorld = new G4PVPlacement(0, G4ThreeVector(), fLogicWorld, "World", 0, false, 0, checkOverlaps);
	
	// volume positions
	G4ThreeVector pos_detector = G4ThreeVector(0, 0, -.5*fDetectorThickness + z_kathode);
	G4ThreeVector pos_coating = G4ThreeVector(0, 0, .5*fCoatingThickness + z_kathode);
	G4ThreeVector pos_kathode = G4ThreeVector(0, 0, fCoatingThickness+.5*fKaptonThickness + z_kathode);

	// Kathode
	G4double sizeX_kathode = sizeX_world, sizeY_kathode = sizeY_world;
	G4Material* mat_kapton = nist->FindOrBuildMaterial("G4_KAPTON");

	G4Box* solid_kathode = new G4Box("Kathode", .5*sizeX_kathode, .5*sizeY_kathode, .5*fKaptonThickness);
	fLogicKathode = new G4LogicalVolume(solid_kathode, mat_kapton, "Kathode");
	G4VisAttributes* visatt_kathode = new G4VisAttributes(G4Colour(1., .64, .08, .5));
	//visatt_kathode->SetForceWireframe(true);
	fLogicKathode->SetVisAttributes(visatt_kathode);
	fPhysKathode = new G4PVPlacement(0, pos_kathode, fLogicKathode, "Kathode", fLogicWorld, false, 0, checkOverlaps);

	// Coating
	G4double sizeX_coating = sizeX_kathode, sizeY_coating = sizeY_kathode;
	G4Material* mat_coating;
	if (fCoatingMaterial) mat_coating = fCoatingMaterial;
	else mat_coating = nist->FindOrBuildMaterial("G4_Au");

	G4Box* solid_coating = new G4Box("Coating", .5*sizeX_coating, .5*sizeY_coating, .5*fCoatingThickness);
	fLogicCoating = new G4LogicalVolume(solid_coating, mat_coating, "Coating");
	G4VisAttributes* visatt_coating = new G4VisAttributes(G4Colour(1., 1., 0., .5));
	//visatt_coating->SetForceWireframe(true);
	fLogicCoating->SetVisAttributes(visatt_coating);
	fPhysCoating = new G4PVPlacement(0, pos_coating, fLogicCoating, "Coating", fLogicWorld, false, 0, checkOverlaps);

	// Detector
	G4double sizeX_detector = sizeX_world, sizeY_detector = sizeY_world;
	G4Material* mat_detector;
	if (fDetectorMaterial) mat_detector = fDetectorMaterial;
	else mat_detector = nist->FindOrBuildMaterial("G4_Ar");

	G4Box* solid_detector = new G4Box("Detector", .5*sizeX_detector, .5*sizeY_detector, .5*fDetectorThickness);
	fLogicDetector = new G4LogicalVolume(solid_detector, mat_detector, "Detector");
	G4VisAttributes* visatt_detector = new G4VisAttributes(G4Colour(1., 1., 1.));
	visatt_detector->SetForceWireframe(true);
	fLogicDetector->SetVisAttributes(visatt_detector);
	fPhysDetector = new G4PVPlacement(0, pos_detector, fLogicDetector, "Detector", fLogicWorld, false, 0, checkOverlaps);

	return fPhysWorld;
}

void DetectorConstruction::SetKaptonThickness(G4double val) {
	if (fPhysWorld) {
		G4Exception ("DetectorConstruction::SetKaptonThickness()", "MM", JustWarning, "Attempt to change already constructed geometry is ignored");
	} else {
		fKaptonThickness = val;
	}
}

void DetectorConstruction::SetCoatingThickness(G4double val) {
	if (fPhysWorld) {
		G4Exception ("DetectorConstruction::SetCoatingThickness()", "MM", JustWarning, "Attempt to change already constructed geometry is ignored");
	} else {
		fCoatingThickness = val;
	}
}

void DetectorConstruction::SetDetectorThickness(G4double val) {
	if (fPhysWorld) {
		G4Exception ("DetectorConstruction::SetDetectorThickness()", "MM", JustWarning, "Attempt to change already constructed geometry is ignored");
	} else {
		fDetectorThickness = val;
	}
}

void DetectorConstruction::SetCoatingMaterial(const G4String& name) {
	G4Material* mat = G4Material::GetMaterial(name, false);

	if(!mat) mat = G4NistManager::Instance()->FindOrBuildMaterial(name);

	if (mat) {
		G4cout << "### New coating material: " << mat->GetName() << G4endl;
		fCoatingMaterial = mat;
		if (fLogicCoating) {
	    	fLogicCoating->SetMaterial(mat); 
	    	G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    	}
	}
}

void DetectorConstruction::SetDetectorMaterial(const G4String& name) {
	G4Material* mat = G4Material::GetMaterial(name, false);

	if(!mat) mat = G4NistManager::Instance()->FindOrBuildMaterial(name);

	if (mat) {
		G4cout << "### New detector material: " << mat->GetName() << G4endl;
		fDetectorMaterial = mat;
		if (fLogicDetector) {
	    	fLogicDetector->SetMaterial(mat); 
	    	G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    	}
	}
}

void DetectorConstruction::SetPairEnergy(G4double val) {
  if(val > 0.0) fCoatingMaterial->GetIonisation()->SetMeanEnergyPerIonPair(val);
}