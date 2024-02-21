//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jan 18 15:07:04 2024 by ROOT version 6.20/07
// from TTree Edep/Edep
// found on file: kaon_12.root
//////////////////////////////////////////////////////////

#ifndef Edep_h
#define Edep_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class Edep {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        edep;
   Double_t        cell_no;
   Double_t        event_id;

   // List of branches
   TBranch        *b_edep;   //!
   TBranch        *b_cell_no;   //!
   TBranch        *b_event_id;   //!

   Edep(TTree *tree=0);
   virtual ~Edep();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef Edep_cxx
Edep::Edep(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("kaon_12.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("kaon_12.root");
      }
      f->GetObject("Edep",tree);

   }
   Init(tree);
}

Edep::~Edep()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t Edep::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t Edep::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void Edep::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("edep", &edep, &b_edep);
   fChain->SetBranchAddress("cell_no", &cell_no, &b_cell_no);
   fChain->SetBranchAddress("event_id", &event_id, &b_event_id);
   Notify();
}

Bool_t Edep::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void Edep::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t Edep::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef Edep_cxx
