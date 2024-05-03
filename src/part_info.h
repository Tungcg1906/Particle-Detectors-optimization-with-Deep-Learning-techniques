//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Jan 18 15:06:34 2024 by ROOT version 6.20/07
// from TTree part_info/part_info
// found on file: kaon_12.root
//////////////////////////////////////////////////////////

#ifndef part_info_h
#define part_info_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class part_info {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        event_id;
   Double_t        part_id;
   Double_t        track_id;
   Double_t        parent_id;
   Double_t        mom;
   Double_t        edepo;
   Double_t        deltae;
   Double_t        global_t;
   Double_t        cublet_idx;
   Double_t        cell_in_cub;

   // List of branches
   TBranch        *b_event_id;   //!
   TBranch        *b_part_id;   //!
   TBranch        *b_track_id;   //!
   TBranch        *b_parent_id;   //!
   TBranch        *b_mom;   //!
   TBranch        *b_edepo;   //!
   TBranch        *b_deltae;
   TBranch        *b_global_t;
   TBranch        *b_cublet_idx;
   TBranch        *b_cell_in_cub;

   part_info(TTree *tree=0);
   virtual ~part_info();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef part_info_cxx
part_info::part_info(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("hadron.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("hadron.root");
      }
      f->GetObject("part_info",tree);

   }
   Init(tree);
}

part_info::~part_info()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t part_info::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t part_info::LoadTree(Long64_t entry)
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

void part_info::Init(TTree *tree)
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

   fChain->SetBranchAddress("event_id",      &event_id,     &b_event_id);
   fChain->SetBranchAddress("part_id",       &part_id,      &b_part_id);
   fChain->SetBranchAddress("track_id",      &track_id,     &b_track_id);
   fChain->SetBranchAddress("parent_id",     &parent_id,    &b_parent_id);
   fChain->SetBranchAddress("mom",           &mom,          &b_mom);
   fChain->SetBranchAddress("edepo",         &edepo,        &b_edepo);
   fChain->SetBranchAddress("deltae",        &deltae,       &b_deltae);
   fChain->SetBranchAddress("global_t",      &global_t,     &b_global_t);
   fChain->SetBranchAddress("cublet_idx",    &cublet_idx,   &b_cublet_idx);
   fChain->SetBranchAddress("cell_in_cub",   &cell_in_cub,  &b_cell_in_cub);
   Notify();
}

Bool_t part_info::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void part_info::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t part_info::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef /lustre/cmswork/xnguyen/part_info_cxx
