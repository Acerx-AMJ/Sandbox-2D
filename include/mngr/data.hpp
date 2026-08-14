#pragma once
#include "SRU/file.hpp"

void loadData();
void loadPrepass(std::vector<Header> &blockHeaders, std::vector<Header> &liquidHeaders, std::vector<Header> &furnitureHeaders, std::vector<Header> &itemHeaders, std::vector<Header> &dropHeaders);

void loadBlockData(std::vector<Header> &headers);
void loadLiquidData(std::vector<Header> &headers);
void loadFurnitureData(std::vector<Header> &headers);
void loadItemData(std::vector<Header> &headers);
void loadDropTableData(std::vector<Header> &headers);
