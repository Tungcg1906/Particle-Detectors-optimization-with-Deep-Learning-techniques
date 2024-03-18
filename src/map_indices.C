template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries) {

  std::map<int, std::vector<UInt_t> > map_idx = {};
  int prev_id = -1;
  for (int j = 0; j < entries; j++) {
    event->GetEntry(j);
    int curr_id = event->event_id;
    // check if new evt index
    if (map_idx.find(curr_id) == map_idx.end()) {
      std::vector<UInt_t> v_temp = {};
      map_idx[curr_id] = v_temp;
    }
    // check if change point
    if (prev_id != curr_id) {
      map_idx[curr_id].push_back(j);
      if (prev_id != -1) {
        map_idx[prev_id].push_back(j-1);
      }
      prev_id = curr_id;
    }
  }
  map_idx[prev_id].push_back(entries-1);

  return(map_idx);
}




std::map<int, std::vector<UInt_t> > m_idx_edep = map_indices(  event,   entries);
  std::cout << "First index map complete" << std::endl;
  std::map<int, std::vector<UInt_t> > m_idx_pi   = map_indices(pievent, pientries);
  std::cout << "Second index map complete" << std::endl;

  int ievent = 0;
  int epsilon = 1;
  bool data = false;
  do {
    // check if event index is present
    if (m_idx_edep.find(ievent) == m_idx_edep.end() ||
        m_idx_pi.find(ievent)   == m_idx_pi.end()) {
      continue;
    }

    e_in_cell.clear();
    cell_idx.clear();
    eventID.clear();
    pipdg.clear();
    picell.clear();
    pimom.clear();

    for (int j = 0; j < m_idx_edep[ievent].size(); j+=2) {
      for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j+1]+1; k++) {
        event->GetEntry(k);
        // Add the current entry to vectors
        e_in_cell.push_back(event->edep);
        cell_idx.push_back(event->cell_no);
        eventID.push_back(event->event_id);
      }
    } // end of for loop

    for (int j = 0; j < m_idx_pi[ievent].size(); j+=2) {
      for (int k = m_idx_pi[ievent][j]; k < m_idx_pi[ievent][j+1]+1; k++) {
        pievent->GetEntry(k);
        // Add the current entry to vectors
        pipdg.push_back(pievent->pdg_id);
        double x = pievent->pos_x;
        double y = pievent->pos_y;
        double z = pievent->pos_z;
        int ix = (x + 150 - epsilon) / 3;
        int iy = (-y + 150 - epsilon) / 3;
        int iz = (z - epsilon) / 12;
        picell.push_back(ix + 100 * iy + 10000 * iz);
        pimom.push_back(pievent->mom);
      }
    } // end of for loop

    // Fill the new Ntuples
    fill_n_tuple (outputTree);
