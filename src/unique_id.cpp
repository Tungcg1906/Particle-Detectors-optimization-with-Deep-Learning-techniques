double entry;
Edep->SetBranchAddress("event_id", &entry);
vector<double> unique_id;
for (int i = 0; i < Edep->GetEntries(); i++) {
    Edep->GetEntry(i);
    bool check = 1;
    for (int j = 0; j < unique_id.size(); j++) {
        if (entry == unique_id[j]) {
            check = 0;
            break;
        }
    }
    if (check) {
        unique_id.push_back(entry);
    }
}