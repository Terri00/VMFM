#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <regex>
#include <set>

#include <chrono>

#include "Util.h"


namespace kv
{
	const std::regex reg_kv("(\"([^=\"]*)\")|([^=\\s]+)");

	class DataBlock
	{
	private:
		std::map<int, std::string> Insertionorder;
	public:
		std::string name = "";
		DataBlock* parentBlock;
		std::vector<DataBlock> SubBlocks;
		std::unordered_map<std::string, std::string> Values;
		
		//Flag weather this datablock should be written or skipped
		bool flag_for_serialize = true;
		int db_order_identifier;

		DataBlock() {}

		DataBlock(std::istringstream* stream, DataBlock* parent, int* c_db_identier, std::string name = "") {
			this->name = sutil::trim(name);
			this->db_order_identifier = (*c_db_identier)++;

			int kvid = 0;
			std::string line, prev = "";
			while (std::getline(*stream, line)) {
				line = split(line, "//")[0];

				if (line.find("{") != std::string::npos) {
					std::string pname = prev;
					prev.erase(std::remove(prev.begin(), prev.end(), '"'), prev.end());
					this->SubBlocks.push_back(DataBlock(stream, this, c_db_identier, pname));
					continue;
				}
				if (line.find("}") != std::string::npos) {
					return;
				}

#ifdef _DEBUG
				// Regex is so fucking slow in debug mode its unreal
				// Rather have it mess up than take 10 hours

				std::vector<std::string> s1 = split(line, '"');
				std::vector<std::string> strings;

				if (s1.size() >= 3)
				{
					strings.push_back(s1[1]);
					strings.push_back(s1[3]);
				}
#endif 

#ifndef _DEBUG
				std::vector<std::string> strings = sutil::regexmulti(line, reg_kv);
#endif

				for (int i = 0; i < strings.size(); i++) {
					strings[i] = sutil::removeChar(strings[i], '"');
				}

				if (strings.size() == 2) {
					this->Values.insert({ strings[0], strings[1] });
					this->Insertionorder.insert({ kvid++, strings[0] });
				}

				prev = line;
			}
		}

		void UpdateForMarked(std::map<int, int>* hitlist) {
			this->flag_for_serialize = hitlist->count(this->db_order_identifier) == 0 ? true : false; //Check if on hitlist
			for (int i = 0; i < this->SubBlocks.size(); i++) //Update children
				this->SubBlocks[i].UpdateForMarked(hitlist);
		}

		//Collect all the ID values from blocks
		void CollectIDs(std::vector<int>* ids){
			if (this->Values.count("id") == 1) ids->push_back((int)::atof(this->Values["id"].c_str()));
			for (int i = 0; i < this->SubBlocks.size(); i++) this->SubBlocks[i].CollectIDs(ids);
		}

		void RemapIDs(std::map<int, int>* remapTable){
			if (this->Values.count("id") == 1) this->Values["id"] = std::to_string((*remapTable)[(int)::atof(this->Values["id"].c_str())]);

			//Update side reference lists
			if(this->Values["classname"] == "info_overlay")
			if (this->Values.count("sides") == 1) {
				std::vector<std::string> vals = split(this->Values["sides"], ' ');

				std::string build = "";
				for (int i = 0; i < vals.size(); i++) {
					build += std::to_string((*remapTable)[(int)::atof(vals[i].c_str())]);
					if (i != vals.size() - 1) build += " ";
				}

				this->Values["sides"] = build;
			}

			for (int i = 0; i < this->SubBlocks.size(); i++) this->SubBlocks[i].RemapIDs(remapTable);
		}

		void Serialize(std::ofstream& stream, int depth = 0)
		{
			if (!this->flag_for_serialize) return; //Skip this node branch completely if this datablock isn't flagged to be written...

			//Build indentation levels
			std::string indenta = "";
			for (int i = 0; i < depth; i++)
				indenta += "\t";
			std::string indentb = indenta + "\t";

			if (depth >= 0)
				stream << indenta << this->name << std::endl << indenta << "{" << std::endl;

			//Write kvs
			for (auto const& x : this->Insertionorder)
			{
				stream << indentb << "\"" << x.second << "\" \"" << this->Values[x.second] << "\"" << std::endl;
			}

			//Write subdata recursively
			for (int i = 0; i < this->SubBlocks.size(); i++){
				this->SubBlocks[i].Serialize(stream, depth + 1);
			}

			if (depth >= 0)
				stream << indenta << "}" << std::endl;
		}

		//Scan for sub block with name
		DataBlock* GetFirstByName(std::string _name) {
			for (int i = 0; i < this->SubBlocks.size(); i++) {
				if (_name == this->SubBlocks[i].name)
					return &this->SubBlocks[i];
			}

			return NULL;
		}

		//Gets all sub blocks by type
		std::vector<DataBlock> GetAllByName(std::string _name) {
			std::vector<DataBlock> c;

			for (int i = 0; i < this->SubBlocks.size(); i++) {
				if (_name == this->SubBlocks[i].name)
					c.push_back(this->SubBlocks[i]);
			}

			return c;
		}
	};

	class FileData
	{
	private:
		int c_db_identifier = 0;
	public:
		DataBlock headNode;

		FileData(std::string filestring)
		{
			std::istringstream sr(filestring);

			auto start = std::chrono::high_resolution_clock::now();

			this->headNode = DataBlock(&sr, &this->headNode, &c_db_identifier);
			std::cout << "Unique order identifiers: " << c_db_identifier << std::endl;


			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			std::cout << "KV Read time: " << milliseconds << "ms" << std::endl;
		}

		FileData()
		{

		}

		std::map<int, int> getRemapTable(FileData* dataB, int* fixups){
			//Collect IDs from both
			std::vector<int> _originalIDs; this->headNode.CollectIDs(&_originalIDs);
			std::vector<int> _targetIDs; dataB->headNode.CollectIDs(&_targetIDs);

			std::set<int> orginalIds(std::make_move_iterator(_originalIDs.begin()), std::make_move_iterator(_originalIDs.end()));
			std::set<int> targetIds(std::make_move_iterator(_targetIDs.begin()), std::make_move_iterator(_targetIDs.end()));
			int _fixups = 0;
			
			//Create Remap table
			std::map<int, int> remapIds;

			for (auto it = targetIds.begin(); it != targetIds.end(); it++){
				if (orginalIds.count(*it) == 0) {
					orginalIds.insert(*it);
					//Remap back itself since there was no collision
					remapIds.insert({ *it, *it }); //Create remap instruction
					continue;
				}; //Skip availible ids

				int newId = *it;

				while (orginalIds.count(newId) == 1) {
					newId++;
				}

				orginalIds.insert(newId);
				remapIds.insert({ *it, newId }); //Create remap instruction
				_fixups++;
			}

			*fixups = _fixups;

			return remapIds;
		}

		~FileData() {}
	};
}