#include "PrtPixelSD.h"

#include "PrtEvent.h"
#include "PrtPrizmHit.h"
#include "PrtBarHit.h"

#include "PrtRunAction.h"
#include "PrtManager.h"

#include <Geant4/G4HCofThisEvent.hh>
#include <Geant4/G4Step.hh>
#include <Geant4/G4ThreeVector.hh>
#include <Geant4/G4SDManager.hh>
#include <Geant4/G4ios.hh>
#include <Geant4/G4RunManager.hh>
#include <TVector3.h>


PrtPixelSD::PrtPixelSD( const G4String& name, 
			const G4String& hitsCollectionName,
			G4int nofCells)
  : G4VSensitiveDetector(name)
{
  collectionName.insert(hitsCollectionName);
}

PrtPixelSD::~PrtPixelSD() 
{ 
}

void PrtPixelSD::Initialize(G4HCofThisEvent* hce){

 // TTree *gTree = new TTree("Prt","Prototype hits tree");
 // Event *fHit = 0;
 // gTree->Branch("event", "Event", &event, 64000, 0);

  // // Create hits collection
  // fHitsCollection 
  //   = new B4cCalorHitsCollection(SensitiveDetectorName, collectionName[0]); 

  // // Add this collection in hce
  // G4int hcID 
  //   = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  // hce->AddHitsCollection( hcID, fHitsCollection ); 

  // // Create hits
  // // fNofCells for cells + one more for total sums 
  // for (G4int i=0; i<fNofCells+1; i++ ) {
  //   fHitsCollection->insert(new B4cCalorHit());
  // }
 
  //PrtManager::Instance()->AddEvent(PrtEvent());
}

G4bool PrtPixelSD::ProcessHits(G4Step* step, G4TouchableHistory* hist){  
  // // energy deposit
  // G4double edep = step->GetTotalEnergyDeposit();
  
  // // step length
  // G4double stepLength = 0.;
  // if ( step->GetTrack()->GetDefinition()->GetPDGCharge() != 0. ) {
  //   stepLength = step->GetStepLength();
  // }

  // if ( edep==0. && stepLength == 0. ) return false;     

  if(step == 0) return false;
 
  //G4ThreeVector translation = hist->GetTranslation();
  //G4ThreeVector localpos = step->GetPreStepPoint()->GetPhysicalVolume()->GetObjectTranslation();
  G4TouchableHistory* touchable = (G4TouchableHistory*)(step->GetPostStepPoint()->GetTouchable());

  // Get cell id 
  G4int layerNumber = touchable->GetReplicaNumber(0);
  //G4cout<< " PixelId = "<<layerNumber << G4endl;
  G4Track* track = step->GetTrack();
  const G4DynamicParticle* dynParticle = track->GetDynamicParticle();
  G4ParticleDefinition* particle = dynParticle->GetDefinition();  
  G4String ParticleName = particle->GetParticleName();
  
  G4ThreeVector globalpos = step->GetPostStepPoint()->GetPosition();
  G4ThreeVector localpos = touchable->GetHistory()->GetTopTransform().TransformPoint(globalpos);
  G4ThreeVector translation = touchable->GetHistory()->GetTopTransform().Inverse().TransformPoint(G4ThreeVector(0,0,0));
  G4ThreeVector inPrismpos = touchable->GetHistory()->GetTransform( 1 ).TransformPoint(globalpos);
  G4ThreeVector g4mom = track->GetVertexMomentumDirection();//GetMomentum();
  G4ThreeVector g4pos = track->GetVertexPosition();
 
  TVector3 globalPos(inPrismpos.x(),inPrismpos.y(),inPrismpos.z());
  //TVector3 globalPos(globalpos.x(),globalpos.y(),globalpos.z());
  TVector3 localPos(localpos.x(),localpos.y(),localpos.z());
  TVector3 digiPos(translation.x(),translation.y(),translation.z());
  TVector3 momentum(g4mom.x(),g4mom.y(),g4mom.z());
  TVector3 position(g4pos.x(),g4pos.y(),g4pos.z());

  // information from prizm
  G4SDManager* fSDM = G4SDManager::GetSDMpointer();
  G4RunManager* fRM = G4RunManager::GetRunManager();
  G4int collectionID = fSDM->GetCollectionID("PrizmHitsCollection");
  const G4Event* currentEvent = fRM->GetCurrentEvent();
  G4HCofThisEvent* HCofEvent = currentEvent->GetHCofThisEvent();
  PrtPrizmHitsCollection* prizmCol = (PrtPrizmHitsCollection*)(HCofEvent->GetHC(collectionID));
  double time = step->GetPreStepPoint()->GetLocalTime();
  
  Long_t pathId = 0;
  Int_t refl=0;
  Int_t prizmId=-1;
  
  for (unsigned int i=0;i<prizmCol->entries();i++){
    PrtPrizmHit* phit = (*prizmCol)[i];    
    if(phit->GetTrackID()==track->GetTrackID()) {
      if(PrtManager::Instance()->GetRunType()==5 && phit->GetNormalId()==-5){
	momentum.SetXYZ(phit->GetPos().x(),phit->GetPos().y(),phit->GetPos().z());
	time -= phit->GetEdep();
      }
      if(phit->GetNormalId()>0){
	++refl;
	if(PrtManager::Instance()->GetEvType()==0 && refl==1) continue;
	pathId = pathId*10+phit->GetNormalId();	
      }
    }
  }
  
  // // information from bar
  // G4int collectionID_bar = fSDM->GetCollectionID("BarHitsCollection");
  // PrtBarHitsCollection* barCol = (PrtBarHitsCollection*)(HCofEvent->GetHC(collectionID_bar));  
  // std::cout<<"==================== barCol->entries() "<<barCol->entries()<<std::endl;
  // G4ThreeVector bmom1;
  // for (G4int i=0;i<barCol->entries();i++){
  //   PrtBarHit* phit = (*barCol)[i];
  //   G4ThreeVector bmom = phit->GetMom();
  //   if(i==0)  bmom1 = phit->GetMom();
  //   std::cout<<i<<" bmom "<<bmom<< " "<<bmom.angle(bmom1) <<std::endl;
  //   // if(bmom.mag()>1000){ 
  //   //   PrtHit hit;
  //   //   hit.SetLeadTime(bmom.angle(bmom1)*1000);    
  //   //   PrtManager::Instance()->AddHit(hit);
  //   // }
  // }
  
  PrtHit hit;
  //hit.SetPrizmId(prizmId);
  int mcp = touchable->GetReplicaNumber(1);
  int pix = touchable->GetReplicaNumber(0);
  hit.SetMcpId(mcp);
  hit.SetPixelId(pix);
  hit.SetChannel(300*mcp+pix);
  hit.SetGlobalPos(globalPos);
  hit.SetLocalPos(localPos);
  hit.SetDigiPos(digiPos);
  hit.SetPosition(position);
  hit.SetMomentum(momentum);
  hit.SetParticleId(track->GetTrackID());
  hit.SetParentParticleId(track->GetParentID());
  hit.SetNreflectionsInPrizm(refl);
  hit.SetPathInPrizm(pathId);
  //hit.SetCherenkovMC(PrtManager::Instance()->GetCurrentCherenkov());
  // time since track created
  hit.SetLeadTime(time);
  Double_t wavelength = 1.2398/(track->GetMomentum().mag()*1E6)*1000;
  hit.SetTotTime(wavelength); //set photon wavelength

  // time since event created
  //hit.SetTrailTime(0,step->GetPreStepPoint()->GetGlobalTime()*1000); 
 
  PrtManager::Instance()->AddHit(hit);
  
  // // Get hit accounting data for this cell
  // B4cCalorHit* hit = (*fHitsCollection)[layerNumber];
  // if ( ! hit ) {
  //   G4ExceptionDescription msg;
  //   msg << "Cannot access hit " << layerNumber; 
  //   G4Exception("PrtPixelSD::ProcessHits()",
  //     "MyCode0004", FatalException, msg);
  // }         

  // // Get hit for total accounting
  // B4cCalorHit* hitTotal 
  //   = (*fHitsCollection)[fHitsCollection->entries()-1];
  
  // // Add values
  // hit->Add(edep, stepLength);
  // hitTotal->Add(edep, stepLength); 
      
  return true;
}

void PrtPixelSD::EndOfEvent(G4HCofThisEvent*)
{ 
  G4int eventNumber = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
  int runtype = PrtManager::Instance()->GetRunType();
  if(eventNumber%1==0 && (runtype==0 || runtype==10)) std::cout<<"Event # "<<eventNumber <<std::endl;
  if(eventNumber%1000==0 && runtype != 0  && runtype != 10) std::cout<<"Event # "<<eventNumber <<std::endl;
  PrtManager::Instance()->Fill();
  
}

