#pragma once
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <sstream>
#include <vector>
#include <map>
#include <regex>

#include "Util.h"
#include "ValveKeyValue.h"
#include "Plane.h"
#include "Mesh.h"
#include "ConvexPolytopes.h"
#include "BoundsSelector.h"

#include <chrono>

namespace vmf_parse {
	//Pass Vector3
	bool Vector3f(std::string str, glm::vec3* vec)
	{
		str = sutil::removeChar(str, '(');
		str = sutil::removeChar(str, ')');

		std::vector<std::string> elems = split(str, ' ');
		std::vector<float> pelems;

		for (int i = 0; i < elems.size(); i++) {
			std::string f = sutil::trim(elems[i]);

			//TODO: error check against invalid values here
			float e = ::atof(f.c_str());
			pelems.push_back(e);
		}

		if (pelems.size() == 3) {
			*vec = glm::vec3(pelems[0], pelems[1], pelems[2]);
			return true;
		}

		return false;
	}

	//Parse plane from standard 3 point notation (ax, ay, az) (bx, by, bz) ...
	bool plane(std::string str, Plane* plane)
	{
		std::vector<std::string> points = split(str, '(');

		if (points.size() != 4) { return false; }

		glm::vec3 A, B, C;

		if (!(Vector3f(points[1], &A) && Vector3f(points[2], &B) && Vector3f(points[3], &C))) {
			return false;
		}

		*plane = Plane(A, B, C);

		return true;
	}
}

namespace vmf {
	struct Side {
		int ID;
		std::string texture;
		Plane plane;
	};

	struct Solid {
		int fileorder_id;
		int ID;
		std::vector<Side> faces;
		glm::vec3 color;
		glm::vec3 origin;
		bool hidden = false;

		Mesh* mesh;
	};

	struct Entity {
		int fileorder_id;
		int ID;
		std::string classname;
		glm::vec3 origin;

		bool hidden = false;
	};

	class vmf {
	private:
	public:
		kv::FileData internal;
		std::vector<Mesh> meshes;
		std::vector<Solid> solids;
		std::vector<Entity> entities;

		vmf(std::string path)
		{
			std::ifstream ifs(path);
			if (!ifs) {
				std::cout << "Could not open file... " << path << std::endl;
				throw std::exception("File read error");
			}

			std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

			kv::FileData data(str);

			this->internal = data;

			//Process solids list
			std::vector<kv::DataBlock> SolidList = data.headNode.GetFirstByName("world")->GetAllByName("solid");
			for (int i = 0; i < SolidList.size(); i++)
			{
				kv::DataBlock cBlock = SolidList[i];

				Solid solid;
				bool valid = true;

				std::vector<kv::DataBlock> Sides = cBlock.GetAllByName("side");
				for (int j = 0; j < Sides.size(); j++)
				{
					kv::DataBlock cSide = Sides[j];

					Side side;
					side.ID = ::atof(cSide.Values["id"].c_str());
					side.texture = cSide.Values["material"];

					Plane plane;
					if (!vmf_parse::plane(cSide.Values["plane"], &plane))
					{
						valid = false; break;
					}

					side.plane = plane;

					solid.faces.push_back(side);
				}

				glm::vec3 color;
				if (vmf_parse::Vector3f(cBlock.GetFirstByName("editor")->Values["color"], &color))
					solid.color = glm::vec3(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
				else
					solid.color = glm::vec3(1, 0, 0);

				solid.fileorder_id = cBlock.db_order_identifier;
				this->solids.push_back(solid);
			}

			//Process entities list
			std::vector<kv::DataBlock> EntitiesList = data.headNode.GetAllByName("entity");
			for (int i = 0; i < EntitiesList.size(); i++) {
				kv::DataBlock block = EntitiesList[i];

				//if (block.Values["classname"] != "prop_static") continue; //Skip anything else than prop static for now

				//Check wether origin can be resolved for entity
				if ((block.GetFirstByName("solid") == NULL) && (block.Values.count("origin") == 0)) {
					std::cout << "Origin could not be resolved for entity with ID " << block.Values["id"]; continue;
				}

				Entity ent;
				ent.classname = block.Values["classname"];
				ent.ID = (int)::atof(block.Values["id"].c_str());

				glm::vec3 loc = glm::vec3(); 
				vmf_parse::Vector3f(block.Values["origin"], &loc);

				if (block.GetFirstByName("solid") != NULL) {
					//Get all solids
					std::vector<kv::DataBlock> _solids = block.GetAllByName("solid");
					std::vector<Solid> _solids_ent;
					for (int i = 0; i < _solids.size(); i++)
					{
						kv::DataBlock cBlock = _solids[i];

						Solid solid;
						bool valid = true;

						std::vector<kv::DataBlock> Sides = cBlock.GetAllByName("side");
						for (int j = 0; j < Sides.size(); j++)
						{
							kv::DataBlock cSide = Sides[j];

							Side side;
							side.ID = ::atof(cSide.Values["id"].c_str());
							side.texture = cSide.Values["material"];

							Plane plane;
							if (!vmf_parse::plane(cSide.Values["plane"], &plane))
							{
								valid = false; break;
							}

							side.plane = plane;

							solid.faces.push_back(side);
						}

						glm::vec3 color;
						if (vmf_parse::Vector3f(cBlock.GetFirstByName("editor")->Values["color"], &color))
							solid.color = glm::vec3(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
						else
							solid.color = glm::vec3(1, 0, 0);

						_solids_ent.push_back(solid);
					}

					//Process convex polytopes
					for (int i = 0; i < _solids_ent.size(); i++) {
						std::vector<Plane> sidePlanes;
						for (int j = 0; j < _solids_ent[i].faces.size(); j++) {
							sidePlanes.push_back(_solids_ent[i].faces[j].plane);
							sidePlanes[j].offset *= 0.01f;
						}

						//Get origin
						Polytope p = Polytope(sidePlanes);
						//this->solids[i].mesh = p.GeneratedMesh;
						ent.origin = (p.NWU + p.SEL) * 0.5f;
					}
				}
				else { //Fallback to Hammers entity origin
					ent.origin = glm::vec3(-loc.x, loc.z, loc.y) * 0.01f;
				}
				
				ent.fileorder_id = block.db_order_identifier;
				this->entities.push_back(ent);
			}
		}

		void ComputeGLMeshes() {
			auto start = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < this->solids.size(); i++) {
				std::vector<Plane> sidePlanes;
				for (int j = 0; j < this->solids[i].faces.size(); j++) {
					sidePlanes.push_back(this->solids[i].faces[j].plane);
					sidePlanes[j].offset *= 0.01f;
				}

				Polytope p = Polytope(sidePlanes);
				this->solids[i].mesh = p.GeneratedMesh;
				this->solids[i].origin = (p.NWU + p.SEL) * 0.5f;
			}

			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

			std::cout << "GL mesh computation: " << milliseconds << "ms" << std::endl;
		}

		void MarkSelected(BoundsSelector* bounds, bool inverse = false)
		{
			for (int i = 0; i < solids.size(); i++)
			{
				solids[i].hidden = !inverse;
				if (solids[i].origin.z > bounds->NW.y) continue;
				if (solids[i].origin.x > bounds->SE.x) continue;

				if (solids[i].origin.z < bounds->SE.y) continue;
				if (solids[i].origin.x < bounds->NW.x) continue;

				solids[i].hidden = inverse;
			}

			for (int i = 0; i < entities.size(); i++)
			{
				entities[i].hidden = !inverse;
				if (entities[i].origin.z > bounds->NW.y) continue;
				if (entities[i].origin.x > bounds->SE.x) continue;

				if (entities[i].origin.z < bounds->SE.y) continue;
				if (entities[i].origin.x < bounds->NW.x) continue;

				entities[i].hidden = inverse;
			}
		}

		void DeflagDatablocksByHidden(int* flaggedC){
			//Build list of fileorderid's to be linked from hidden entities/brushes
			
			int flagged = 0;

			std::map<int, int> ids;
			for (int i = 0; i < this->solids.size(); i++) {
				if (this->solids[i].hidden) {
					ids.insert({ this->solids[i].fileorder_id, 0 });
					flagged++;
				}
			}

			for (int i = 0; i < this->entities.size(); i++) {
				if (this->entities[i].hidden) {
					ids.insert({ this->entities[i].fileorder_id, 0 });
					flagged++;
				}
			}
			
			*flaggedC = flagged;

			//Go through internal datablocks and disable if ID is in the list
			this->internal.headNode.UpdateForMarked(&ids);
		}

		void CopySerialMarkedSolids(vmf* b, int* _copied) {
			std::vector<kv::DataBlock> solidList = b->internal.headNode.GetFirstByName("world")->GetAllByName("solid");
			int copied = 0;
			for (int i = 0; i < solidList.size(); i++) {
				kv::DataBlock db = solidList[i];

				if (!db.flag_for_serialize) continue; //Skip unmarked solids
				this->internal.headNode.GetFirstByName("world")->SubBlocks.push_back(db); //Append block
				copied++;
			}

			*_copied = copied;
		}

		void CopySerialMarkedEntities(vmf* b, int* _copied) {
			std::vector<kv::DataBlock> entList = b->internal.headNode.GetAllByName("entity");
			int copied = 0;
			for (int i = 0; i < entList.size(); i++) {
				kv::DataBlock db = entList[i];

				if (!db.flag_for_serialize) continue; //Skip unmarked ents
				this->internal.headNode.SubBlocks.push_back(db); //Append block
				copied++;
			}

			*_copied = copied;
		}

		void Commit_Merge(vmf* b){
			std::cout << "Merging VMFs" << std::endl;
			auto start = std::chrono::high_resolution_clock::now();

			std::cout << "Marking selected objects for merge (A)... "; // OBJECT DISABLE SERIAL WRITE =====================
			int count = -1;
			this->DeflagDatablocksByHidden(&count); std::cout << " OK (" << count << ")" << std::endl;
			std::cout << "Marking inverse selection for merge (B)... ";
			b->DeflagDatablocksByHidden(&count); std::cout << " OK (" << count << ")" << std::endl;

			std::cout << "Remapping ID collisions... "; // ID REMAPPING ===================================================
			std::map<int, int> idRemapTable = this->internal.getRemapTable(&b->internal, &count);
			b->internal.headNode.RemapIDs(&idRemapTable);
			std::cout << "OK (" << count << ")" << std::endl;

			std::cout << "Merging Solids... "; // SOLIDS CLONE =============================================================
			this->CopySerialMarkedSolids(b, &count);
			std::cout << "OK (" << count << ")" << std::endl;

			std::cout << "Merging Entities... "; // ENTITIES MERGE =========================================================
			this->CopySerialMarkedEntities(b, &count);
			std::cout << "OK (" << count << ")" << std::endl;

			std::cout << "Serializing VMF file... "; // VMF Data Serialization =============================================
			std::ofstream file("../mrg01.vmf"); this->internal.headNode.Serialize(file, -1); file.close();
			std::cout << "OK" << std::endl;

			std::cout << "Done!! Written to 'mrg01.vmf'" << std::endl;

			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

			std::cout << "Merge time: " << milliseconds << "ms" << std::endl;
		}

		~vmf() {
			for (int i = 0; i < this->solids.size(); i++){
				delete this->solids[i].mesh;
				this->solids[i].mesh = NULL;
			}
		}
	};
}