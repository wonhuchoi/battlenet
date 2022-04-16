#include "imgui_system.hpp"
#include "Units/units.hpp"
#include "game_components.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "render.hpp"
#include "render_components.hpp"
#include "tiny_ecs.hpp"
#include "world.hpp"

#include <iostream>
#include <fstream>
#include <string>


// World initialization
ImguiSystem::ImguiSystem(GLFWwindow* window) :
	window(window)
{
	//imgui
	const char* glsl_version = "#version 130";
	//GLFWwindow* window = glfwCreateWindow(500, 500, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	//glfwMakeContextCurrent(world.window);

	ImGui::CreateContext();
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

ImguiSystem::~ImguiSystem()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void drawUnitStat(vec2 pos, std::vector<std::string> desc)
{
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
	ImGui::SetNextWindowSize(ImVec2(0,0));
	ImGui::Begin(desc[0].c_str());
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

	//here we iterate from i = 1 instead of 0, since the 0 one is the title.
	for (unsigned int i = 1; i < desc.size(); i ++)
	{
		ImGui::Text(desc[i].c_str());  
	}
	ImGui::PopTextWrapPos();
	ImGui::End();
}

void classTextHover(ClassType ct) {
	std::string classText = getClassText(ct);
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::Text(classText.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

void ImguiSystem::start(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h) {
	
	{
		ShadedMesh& resource = createButton({ w / 2 - 100, h / 2 + 150 },
			cam_transform, { 200, 100 }, "startButton", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource.texture.texture_id.resource, ImVec2(170, 75))) {
			std::cout << "INTRO\n";
			world.worldState = world.WORLD_INTRO;
			world.notifyObservers();
		}
		ImGui::End();
	}
}

void ImguiSystem::intro(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h) {

	vec2 imgui_pos = { 0,0 };
	{
		imgui_pos = { w / 2 - 450 , h / 2 - 125 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(900, 250));

		ImGui::Begin("Mission Objective", NULL, window_flags);
		ImGui::Text("You are a sentient virus, tasked with infiltrating this unsuspecting PC.");
		ImGui::Text("");
		ImGui::Text("Fight against challenging firewalls and foes, and salvage their resources.");
		ImGui::Text("");
		ImGui::Text("Use them well, as these resources can be used to upgrade your own arsenal.");
		ImGui::Text("");
		ImGui::Text("Good Luck.");
		ImGui::SetWindowFontScale(1.7);
		ImGui::End();

		ShadedMesh& resource = createButton({ w / 2 - 100, h / 2 + 150 },
			cam_transform, { 200, 100 }, "nextButton", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource.texture.texture_id.resource, ImVec2(175, 75))) {
			std::cout << "LEVEL SELECT\n";
			world.worldState = world.WORLD_SELECT;
			world.notifyObservers();
		}
		ImGui::End();
	}
}

void ImguiSystem::levelSelect(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h) {

	{
		ShadedMesh& resource = createButton({ w / 4 - 200, h / 2 - 150 },
			cam_transform, { 300, 300 }, "campaign", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource.texture.texture_id.resource, ImVec2(275.f, 250.f))) {
			std::cout << "CAMPAIGN\n";
			world.create_level(world.WORLD_CAMPAIGN);
		}
		ImGui::End();

		ShadedMesh& resource2 = createButton({ (w * 0.75) - 100, h / 2 - 150 },
			cam_transform, { 300, 300 }, "playground", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource2.texture.texture_id.resource, ImVec2(275.f, 250.f))) {
			std::cout << "PLAYGROUND" << std::endl;
			world.create_level_helper(true);
		}
		ImGui::End();

		ShadedMesh& resource3 = createButton({ (w / 2) - 150, h / 2 },
			cam_transform, { 300, 300 }, "tutorial", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource3.texture.texture_id.resource, ImVec2(275.f, 250.f))) {
			// tutorial
			world.create_level(world.WORLD_TUTORIAL);
			world.tutorialStep = 0;
		}
		ImGui::End();
	}
}

void ImguiSystem::tutorial(WorldSystem& world, PhaseSystem& phase, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h,
	float elapsed_ms) {

	// Check for mouse hover over all entities with ImGuiDesc component
	auto& IG_container = ECS::registry<ImGuiDesc>;
	// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
	for (unsigned int i = 0; i < IG_container.components.size(); i++)
	{
		ImGuiDesc& ig_desc = IG_container.components[i];
		ECS::Entity entity = IG_container.entities[i];
		auto& motion = ECS::registry<Motion>.get(entity);

		if (ig_desc.activated)
		{
			vec2 imgui_pos = motion.position + ig_desc.rel_position_from_center;
			imgui_pos = imgui_pos += cam_transform;

			drawUnitStat(imgui_pos, ig_desc.description);
		}
	}
	vec2 imgui_pos = { 0,0 };
	{
		// Stage info UI
		imgui_pos = { 750, 35 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		auto& boardInfo = ECS::registry<BoardInfo>.get(world.board);
		ImGui::Begin("Info");
		ImGui::Text("Stage: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.currLevel).c_str());
		ImGui::SameLine();
		ImGui::Text("     Unit Limit: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.unitLimit).c_str());
		ImGui::Text("Gold: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.gold).c_str());
		ImGui::SameLine();
		ImGui::Text("     Units on Board: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.boardUnits).c_str());

		ImGui::End();

		// Bench tooltip
		imgui_pos = { 650, 750 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(50, 50));
		ImGui::Begin("Help");
		HelpMarker("Drag and Drop the Units from this bench to the board");
		ImGui::End();
	}

	// Shop UI
	if (ECS::registry<PlanningPhase>.has(world.board))
	{
		time_taken = 0;
		score.waveEnemies = 0;

		BoardInfo& bi = ECS::registry<BoardInfo>.get(world.board);
		int& gold = bi.gold;
		imgui_pos = { 750, 100 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 450));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Shop");

		ImGui::Text("Click to Purchase Units");
		ShadedMesh& refreshIcon = cache_resource("shop_refresh");
		if (refreshIcon.effect.program.resource == 0) {
			RenderSystem::createSprite(refreshIcon, textures_path("shop_refresh.png"), "textured");
		}
		ImTextureID refreshImage = (ImTextureID)refreshIcon.texture.texture_id.resource;
		if (ImGui::ImageButton(refreshImage, ImVec2(40.f, 40.f))) {
			if (gold - 2 >= 0) {
				gold -= 2;
				world.shuffle_shop();
			}
			else {
				printf("No gold");
			}
		}
		ImGui::SameLine();
		ImGui::Text("Refresh Shop Cost = 2");
		//iterate over the first 4 shuffled units (shop is randomized)
		//Since this is tutorial force the trojan to be the front
		for (int i = 0; i < world.unitVec.size(); i++)
		{
			int utInt = world.unitVec[i];
			UnitType ut = static_cast<UnitType>(utInt);
			if (ut == trojan)
			{
				world.unitVec[i] = world.unitVec[0];
				world.unitVec[0] = utInt;
			}
		}

		Team t = ally;
		for (int i = 0; i < 4; i++)
		{
			int utInt = world.unitVec[i];
			UnitType ut = static_cast<UnitType>(utInt);
			std::string key = teamToString(t) + "_" + unitToString(ut);
			ShadedMesh& resource = cache_resource(key);
			std::vector<vec2>& hull_resource = cache_hull_resource(key);
			// Create sprite if not in cache
			if (resource.effect.program.resource == 0)
				RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured", hull_resource, true);
			// Create hull if not in cache
			if (hull_resource.size() == 0)
				RenderSystem::createHull(hull_resource, textures_path(key + ".png"));

			ImTextureID unitImage = (ImTextureID)resource.texture.texture_id.resource;
			int unitCost = getCostForUnit(ut);
			// Style shop button if it is disabled
			if (world.shopIndexDisabled(i)) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			bool shopButton = ImGui::ImageButton(unitImage, ImVec2(64.f, 64.f));
			if (world.shopIndexDisabled(i)) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			if (shopButton)
			{
				WorldPosition wp = ECS::registry<WorldPosition>.get(world.board);
				if (gold - unitCost < 0) {
					printf("No gold");
				}
				else {
					world.disableShopIndex(i);
					ECS::Entity unit;
					switch (ut) {
					case debugger:
						unit = UnitDebugger::createUnitDebugger(vec2(0, 0), bi, wp, ally);
						break;
					case trojan:
						// For Tutorial (Check if the User purchased this unit)
						if (world.tutorialStep == 1)
						{
							world.tutorialStepDone = true;
						}

						unit = UnitTrojan::createUnitTrojan(vec2(0, 0), bi, wp, ally);
						break;
					case hdd:
						unit = UnitHDD::createUnitHDD(vec2(0, 0), bi, wp, ally);
						break;
					case ddos:
						unit = UnitDDOS::createUnitDDOS(vec2(0, 0), bi, wp, ally);
						break;
					case udp:
						unit = UnitUDP::createUnitUDP(vec2(0, 0), bi, wp, ally);
						break;
					case dhcp:
						unit = UnitDHCP::createUnitDHCP(vec2(0, 0), bi, wp, ally);
						break;
					case wifi:
						unit = UnitWifi::createUnitWifi(vec2(0, 0), bi, wp, ally);
						break;
					case cpu:
						unit = UnitCPU::createUnitCPU(vec2(0, 0), bi, wp, ally);
						break;
					case phishing:
						unit = UnitPhishing::createUnitPhishing(vec2(0, 0), bi, wp, ally);
						break;
					case ransomware:
						unit = UnitRansomware::createUnitRansomware(vec2(0, 0), bi, wp, ally);
						break;
					case ram:
						unit = UnitRAM::createUnitRAM(vec2(0, 0), bi, wp, ally);
						break;
					}
					if (!world.placeUnitOnBench(unit))
					{
						printf("No more space left on the bench");
						//destroy the unit.
						ECS::ContainerInterface::remove_all_components_of(unit);
					}
					else {
						gold = gold - unitCost;
						std::cout << gold << std::endl;
					}
				}
			}

			ImVec2 nextLine = ImGui::GetCursorPos();
			ImGui::SameLine();
			float origCPosX = ImGui::GetCursorPosX();
			float origCPosY = ImGui::GetCursorPosY();
			// Offset amount for text
			float yOffset = 25;
			ImGui::Text("unit type = %s\nunit cost = %d",
				unitToString(ut).c_str(), unitCost);
			// Set cursor position to just below text
			ImGui::SetCursorPos({ origCPosX, origCPosY + yOffset });

			// Draw class images for each unit in the store
			for (ClassType ct : getClassesForUnit(ut)) {
				std::string key = classToString(ct);
				// Get image resource
				ShadedMesh& resource = cache_resource(key);
				if (resource.effect.program.resource == 0) {
					RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured");
				}
				ImTextureID classImage = (ImTextureID)resource.texture.texture_id.resource;
				ImVec2 iconSize = { 32,32 };
				ImGui::Image(classImage, iconSize);
				if (ImGui::IsItemHovered())
				{
					classTextHover(ct);
				}
				// Make all class images go on same line
				ImGui::SameLine();
			}
			// Reset cursor pos to the next position in the shop for next loop
			ImGui::SetCursorPos(nextLine);
		}
		ImGui::End();
	}
	else if (ECS::registry<BattlePhase>.has(world.board) && world.worldState != world.WORLD_PAUSE) {
		time_taken += elapsed_ms;
		total_time += elapsed_ms;
	}

	// Unit class count UI
	imgui_pos = { 1000, 100 };
	imgui_pos += cam_transform;
	ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
	ImGui::SetNextWindowSize(ImVec2(150, 0));
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
	ImGui::Begin("Unit Classes");
	for (int ctInt = software; ctInt < END_CLASSTYPE_ENUM; ctInt++) {
		ClassType ct = static_cast<ClassType>(ctInt);
		std::string key = classToString(ct);
		// Get image resource
		ShadedMesh& resource = cache_resource(key);
		if (resource.effect.program.resource == 0) {
			RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured");
		}
		ImTextureID classImage = (ImTextureID)resource.texture.texture_id.resource;
		ImVec2 iconSize = { 32,32 };
		ImGui::Image(classImage, iconSize);
		if (ImGui::IsItemHovered())
		{
			classTextHover(ct);
		}
		std::string classCountText = std::to_string(world.allyBuffs.classCount[ct]) + "/" + std::to_string(getClassBuffMin(ct));
		ImGui::SameLine();
		ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + (iconSize.y - ImGui::GetFont()->FontSize) / 2 });
		ImGui::Text("%s", classCountText.c_str());
	}
	ImGui::End();

	// Tutorial UI's
	Motion& unitMotion = ECS::registry<Motion>.get(world.player);
	vec2 imgui_tutorial_pos = { unitMotion.position.x + unitMotion.scale.x/2, unitMotion.position.y - unitMotion.scale.y / 2 };
	ImVec2 next_button_size = { 0,0 };

	if (world.tutorialStepDone)
	{
		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(375, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		if (world.tutorialStep == 5)
		{
			ImGui::Text("Enjoy the battle!\n");
		}
		else if (world.tutorialStep == 6)
		{
			ImGui::Text("Great Job! Now you are ready to hack the computer!\n");
			ImGui::Text("Join me now in the 'Campaign' mode to hack the computer with me!\n");

			if (ImGui::Button("Go to Campaign Mode!", next_button_size))
			{
				world.restart();
				phase.begin();
				world.worldState = world.WORLD_SELECT;
				world.notifyObservers();
			}
			ImGui::End();
			return;
		}
		else
		{
			ImGui::Text("Great Job! Let's move onto the next task!\n");
		}
		if (ImGui::Button("Next", next_button_size))
		{
			world.tutorialStep++;
			world.tutorialStepDone = false;
		}
		ImGui::End();
	}
	else if (world.tutorialStep == 0) //Introduction
	{
		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(375, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Hi player, help me hack this computer!\n");
		if (ImGui::Button("Next", next_button_size))
		{
			world.tutorialStep++;
		}
		ImGui::End();
	}
	else if (world.tutorialStep == 1) // Buy Units
	{
		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Let's begin by purchasing some units from the shop\n");
		ImGui::Text("Move your mouse over to the Shop pannel on the right side of the board\n");
		ImGui::Text("Click on the 'trojan' unit to purchase it.\n");
		ImGui::Text("Then a 'trojan' unit will be added to your bench.\n");
		ImGui::Text("Note that your gold decreases by the cost of the unit.\n");
		ImGui::Text("You can earn more gold by picking up coins randomly dropped from dead enemy units.\n");
		ImGui::End();
	}
	else if (world.tutorialStep == 2) // Place Units
	{

		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Even though you now have a 'trojan' unit on your bench,\n");
		ImGui::Text("your 'trojan' unit will not fight in the battle yet\n");
		ImGui::Text("because it is only in your bench.\n");
		ImGui::Text("Make your 'trojan' unit participate in the battle\n");
		ImGui::Text("by dragging the unit onto anywhere on the board.\n");
		ImGui::End();
	}
	else if (world.tutorialStep == 3)// Buffs
	{
		if (world.allyBuffs.classCount[malware] >= getClassBuffMin(malware))
		{
			//task done
			world.tutorialStepDone = true;
		}

		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Let's explore the buff system!\n");
		ImGui::Text("Placing multiple units with the same traits onto the board\n");
		ImGui::Text("activates corresponding buffs to all units on the board.\n");
		ImGui::Text("\n");
		ImGui::Text("Since you already bought 'trojan' unit from this shop,\n");
		ImGui::Text("Click on the 'Refresh Shop' button to get a new random shop with cost of 2 golds.\n");
		ImGui::Text("\n");
		ImGui::Text("Then purchase another 'trojan' unit from the Shop,\n");
		ImGui::Text("and place it on the board to activate 'Malware' buff.\n");
		ImGui::Text("'Malware' buff will increase attack damage of all units!\n");
		ImGui::Text("\n");
		ImGui::Text("Detailed information about each buff\n");
		ImGui::Text("and the number of units required to activate it\n");
		ImGui::Text("can be found on the 'Unit Classes' pannel on the right.\n");
		ImGui::End();
	}
	else if (world.tutorialStep == 4) //Explain mana/skill system
	{
		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Each unit has different abilities and Combat Stats.\n");
		ImGui::Text("You can hover over the units to check its ability, \n");
		ImGui::Text("and its combat statistics real-time during the battle\n");
		if (ImGui::Button("Next", next_button_size))
		{
			world.tutorialStep++;
		}
		ImGui::End();
	}
	else if (world.tutorialStep == 5) //Fight
	{
		if (ECS::registry<BattlePhase>.has(world.board))
		{
			world.tutorialStepDone = true;
		}
		imgui_pos = imgui_tutorial_pos;
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Now we are ready for battle!\n\n");
		ImGui::Text("Press space to move to start the battle!\n\n");
		ImGui::Text("Your units will fight on their own!\n");
		ImGui::End();
	}
	else 
	{
		//Tutorial ended
		return;
	}

	
}

void ImguiSystem::campaign(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h,
	float elapsed_ms) {

	// Check for mouse hover over all entities with ImGuiDesc component
	auto& IG_container = ECS::registry<ImGuiDesc>;
	// for (auto [i, motion_i] : enumerate(motion_container.components)) // in c++ 17 we will be able to do this instead of the next three lines
	for (unsigned int i = 0; i < IG_container.components.size(); i++)
	{
		ImGuiDesc& ig_desc = IG_container.components[i];
		ECS::Entity entity = IG_container.entities[i];
		auto& motion = ECS::registry<Motion>.get(entity);

		if (ig_desc.activated)
		{
			vec2 imgui_pos = motion.position + ig_desc.rel_position_from_center;
			imgui_pos = imgui_pos += cam_transform;

			drawUnitStat(imgui_pos, ig_desc.description);
		}
	}
	vec2 imgui_pos = { 0,0 };
	{
		// Stage info UI
		imgui_pos = { 750, 35 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		auto& boardInfo = ECS::registry<BoardInfo>.get(world.board);
		ImGui::Begin("Info");
		ImGui::Text("Stage: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.currLevel).c_str());
		ImGui::SameLine();
		ImGui::Text("     Unit Limit: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.unitLimit).c_str());
		ImGui::Text("Gold: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.gold).c_str());
		ImGui::SameLine();
		ImGui::Text("     Units on Board: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(boardInfo.boardUnits).c_str());

		ImGui::End();

		// Tutorial UI (Below shop)
		imgui_pos = { 750, 550 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(375, 0));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Tutorial");
		ImGui::Text("Planning Phase\n\n");
		ImGui::Text("Drag and drop your units within the green squares!\n");
		ImGui::Text("Find strategic positioning to defeat the opponents!\n\n");
		ImGui::Text("You can save the current unit positions with M,\n");
		ImGui::Text("and load the saved positions with the N key.\n\n");
		ImGui::Text("Press space to move to the battle phase.\n\n");
		ImGui::Text("Battle Phase\n\n");
		ImGui::Text("Your units will fight on their own!\n");
		ImGui::Text("You win if your units defeat all enemies!\n");
		ImGui::End();

		// Bench tooltip
		imgui_pos = { 650, 750 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(50, 50));
		ImGui::Begin("Help");
		HelpMarker("Drag and Drop the Units from this bench to the board");
		ImGui::End();
	}

	// Shop UI
	if (ECS::registry<PlanningPhase>.has(world.board))
	{
		time_taken = 0;
		score.waveEnemies = 0;

		BoardInfo& bi = ECS::registry<BoardInfo>.get(world.board);
		int& gold = bi.gold;
		imgui_pos = { 750, 100 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(0, 450));
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
		ImGui::Begin("Shop");

		ImGui::Text("Click to Purchase Units");
		ShadedMesh& refreshIcon = cache_resource("shop_refresh");
		if (refreshIcon.effect.program.resource == 0) {
			RenderSystem::createSprite(refreshIcon, textures_path("shop_refresh.png"), "textured");
		}
		ImTextureID refreshImage = (ImTextureID)refreshIcon.texture.texture_id.resource;
		if (ImGui::ImageButton(refreshImage , ImVec2(40.f, 40.f))) {
			if (gold - 2 >= 0) {
				gold -= 2;
				world.shuffle_shop();
			} else {
				printf("No gold");
			}
		}
		ImGui::SameLine();
		ImGui::Text("Refresh Shop Cost = 2");
		//iterate over the first 4 shuffled units (shop is randomized)
		Team t = ally;
		int numUnits = world.in_playground ? world.unitVec.size() : 4;
		for (int i = 0; i < numUnits; i++)
		{
			int utInt = world.unitVec[i];
			UnitType ut = static_cast<UnitType>(utInt);
			std::string key = teamToString(t) + "_" + unitToString(ut);
			ShadedMesh& resource = cache_resource(key);
			std::vector<vec2>& hull_resource = cache_hull_resource(key);
			// Create sprite if not in cache
			if (resource.effect.program.resource == 0)
				RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured", hull_resource, true);
			// Create hull if not in cache
			if (hull_resource.size() == 0)
				RenderSystem::createHull(hull_resource, textures_path(key + ".png"));

			ImTextureID unitImage = (ImTextureID)resource.texture.texture_id.resource;
			int unitCost = getCostForUnit(ut);
			// Style shop button if it is disabled
			if (world.shopIndexDisabled(i)) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			bool shopButton = ImGui::ImageButton(unitImage, ImVec2(64.f, 64.f));
			if (world.shopIndexDisabled(i)) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			if (shopButton)
			{
				WorldPosition wp = ECS::registry<WorldPosition>.get(world.board);
				if (gold - unitCost < 0) {
					printf("No gold");
				}
				else {
					world.disableShopIndex(i);
					ECS::Entity unit;
					switch (ut) {
					case debugger:
						unit = UnitDebugger::createUnitDebugger(vec2(0, 0), bi, wp, ally);
						break;
					case trojan:
						unit = UnitTrojan::createUnitTrojan(vec2(0, 0), bi, wp, ally);
						break;
					case hdd:
						unit = UnitHDD::createUnitHDD(vec2(0, 0), bi, wp, ally);
						break;
					case ddos:
						unit = UnitDDOS::createUnitDDOS(vec2(0, 0), bi, wp, ally);
						break;
					case udp:
						unit = UnitUDP::createUnitUDP(vec2(0, 0), bi, wp, ally);
						break;
					case dhcp:
						unit = UnitDHCP::createUnitDHCP(vec2(0, 0), bi, wp, ally);
						break;
					case wifi:
						unit = UnitWifi::createUnitWifi(vec2(0, 0), bi, wp, ally);
						break;
					case cpu:
						unit = UnitCPU::createUnitCPU(vec2(0, 0), bi, wp, ally);
						break;
					case phishing:
						unit = UnitPhishing::createUnitPhishing(vec2(0, 0), bi, wp, ally);
						break;
					case ransomware:
						unit = UnitRansomware::createUnitRansomware(vec2(0, 0), bi, wp, ally);
						break;
					case ram:
						unit = UnitRAM::createUnitRAM(vec2(0, 0), bi, wp, ally);
						break;
					}
					if (!world.placeUnitOnBench(unit))
					{
						printf("No more space left on the bench");
						//destroy the unit.
						ECS::ContainerInterface::remove_all_components_of(unit);
					}
					else {
						gold = world.in_playground ? gold : (gold - unitCost);
						std::cout << gold << std::endl;
					}
				}
			}

			ImVec2 nextLine = ImGui::GetCursorPos();
			ImGui::SameLine();
			float origCPosX = ImGui::GetCursorPosX();
			float origCPosY = ImGui::GetCursorPosY();
			// Offset amount for text
			float yOffset = 25;
			ImGui::Text("unit type = %s\nunit cost = %d",
				unitToString(ut).c_str(), unitCost);
			// Set cursor position to just below text
			ImGui::SetCursorPos({ origCPosX, origCPosY + yOffset });

			// Draw class images for each unit in the store
			for (ClassType ct : getClassesForUnit(ut)) {
				std::string key = classToString(ct);
				// Get image resource
				ShadedMesh& resource = cache_resource(key);
				if (resource.effect.program.resource == 0) {
					RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured");
				}
				ImTextureID classImage = (ImTextureID)resource.texture.texture_id.resource;
				ImVec2 iconSize = { 32,32 };
				ImGui::Image(classImage, iconSize);
				if (ImGui::IsItemHovered())
				{
					classTextHover(ct);
				}
				// Make all class images go on same line
				ImGui::SameLine();
			}
			// Reset cursor pos to the next position in the shop for next loop
			ImGui::SetCursorPos(nextLine);
		}
		ImGui::End();
	}
	else if (ECS::registry<BattlePhase>.has(world.board) && world.worldState != world.WORLD_PAUSE) {
		time_taken += elapsed_ms;
		total_time += elapsed_ms;
	}

	// Unit class count UI
	imgui_pos = { 1000, 100 };
	imgui_pos += cam_transform;
	ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
	ImGui::SetNextWindowSize(ImVec2(150, 0));
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
	ImGui::Begin("Unit Classes");
	for (int ctInt = software; ctInt < END_CLASSTYPE_ENUM; ctInt++) {
		ClassType ct = static_cast<ClassType>(ctInt);
		std::string key = classToString(ct);
		// Get image resource
		ShadedMesh& resource = cache_resource(key);
		if (resource.effect.program.resource == 0) {
			RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured");
		}
		ImTextureID classImage = (ImTextureID)resource.texture.texture_id.resource;
		ImVec2 iconSize = { 32,32 };
		ImGui::Image(classImage, iconSize);
		if (ImGui::IsItemHovered())
		{
			classTextHover(ct);
		}
		std::string classCountText = std::to_string(world.allyBuffs.classCount[ct]) + "/" + std::to_string(getClassBuffMin(ct));
		ImGui::SameLine();
		ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + (iconSize.y - ImGui::GetFont()->FontSize) / 2 });
		ImGui::Text("%s", classCountText.c_str());
	}
	ImGui::End();
}

void ImguiSystem::victory(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h) {

	vec2 imgui_pos = { 0,0 };
	{
		imgui_pos = { w / 2 - 350 , h / 2 - 125 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(700, 250));

		ImGui::Begin("Status", NULL, window_flags);
		ImGui::Text("Progressing to the next stage...");
		ImGui::Text("All anti-viruses eliminated!");
		ImGui::Text("");
		// TODO - add a score for the user
		// ImGui::Text("Score:");
		// ImGui::Text("");
		ImGui::Text("Enemies Destroyed this Wave: %d", score.waveEnemies);
		ImGui::Text("Breach Duration: %f seconds", time_taken / 1000);
		ImGui::SetWindowFontScale(2);
		ImGui::End();

		ShadedMesh& resource = createButton({ w / 2 - 100, h / 2 + 100 },
			cam_transform, { 200, 100 }, "nextButton", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource.texture.texture_id.resource, ImVec2(175, 75))) {
			world.next_level();
		}
		ImGui::End();
	}
}

void ImguiSystem::defeat(WorldSystem& world, ScoreSystem& score,
	ImGuiWindowFlags window_flags, vec2 cam_transform, int w, int h) {

	vec2 imgui_pos = { 0,0 };
	{
		imgui_pos = { w / 2 - 350 , h / 2 - 125 };
		imgui_pos += cam_transform;
		ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
		ImGui::SetNextWindowSize(ImVec2(700, 250));

		ImGui::Begin("Status", NULL, window_flags);
		ImGui::Text("The target's firewall was too strong...");
		ImGui::Text("");
		// TODO - add a score for the user
		// ImGui::Text("Final Score:");
		// ImGui::Text("");
		ImGui::Text("Total Enemies Destroyed: %d", score.enemiesDefeated);
		ImGui::Text("Total Breach Time: %f seconds", total_time / 1000);
		ImGui::SetWindowFontScale(2);
		ImGui::End();

		ShadedMesh& resource = createButton({ w / 2 - 300, h / 2 + 100 },
			cam_transform, { 200, 100 }, "restartButton", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource.texture.texture_id.resource, ImVec2(175, 75))) {

			world.worldState = world.WORLD_CAMPAIGN;
			if (world.in_playground) {
				world.next_level();
			}
			else {
				ECS::registry<DefeatPhase>.remove(world.board);
				ECS::registry<PlanningPhase>.emplace(world.board);
				ECS::registry<BoardInfo>.get(world.board).gold = world.prevGold;
				world.open_level(ECS::registry<BoardInfo>.get(world.board).currLevel);
			}
		}
		ImGui::End();

		ShadedMesh& resource2 = createButton({ w / 2 + 100, h / 2 + 100 },
			cam_transform, { 200, 100 }, "quitButton", window_flags);
		if (ImGui::ImageButton((ImTextureID)resource2.texture.texture_id.resource, ImVec2(175, 75))) {
			world.restart();
		}
		ImGui::End();
	}
}

ShadedMesh& ImguiSystem::createButton(vec2 imgui_pos, vec2 cam_transform, 
	vec2 window_size, std::string key, ImGuiWindowFlags window_flags) {
	ImGui::SetNextWindowPos(ImVec2(imgui_pos.x, imgui_pos.y));
	ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
	ImGui::Begin(key.c_str(), NULL, window_flags);
	ShadedMesh& resource = cache_resource(key);
	std::vector<vec2>& hull_resource = cache_hull_resource(key);
	// Create sprite if not in cache
	if (resource.effect.program.resource == 0) {
		RenderSystem::createSprite(resource, textures_path(key + ".png"), "textured", hull_resource, true);
	}
	// Create hull if not in cache
	if (hull_resource.size() == 0) {
		RenderSystem::createHull(hull_resource, textures_path(key + ".png"));
	}
	return resource;
}

void ImguiSystem::step(WorldSystem& world, PhaseSystem& phase, ScoreSystem& score, float elapsed_ms)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	vec2 cam_transform = world.get_cam_transform();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (world.worldState == world.WORLD_START) {
		time_taken = 0;
		total_time = 0;
		score.waveEnemies = 0;
		score.enemiesDefeated = 0;
		start(world, score, window_flags, cam_transform, w, h);
	}
	else if (world.worldState == world.WORLD_INTRO) {
		intro(world, score, window_flags, cam_transform, w, h);
	}
	else if (world.worldState == world.WORLD_SELECT) {
		levelSelect(world, score, window_flags, cam_transform, w, h);
	}
	else if (world.modeState == world.WORLD_TUTORIAL) {
		tutorial(world, phase, score, window_flags, cam_transform, w, h, elapsed_ms);
	}
	else if (world.worldState == world.WORLD_CAMPAIGN) {
		campaign(world, score, window_flags, cam_transform, w, h, elapsed_ms);
	}
	else if (world.worldState == world.WORLD_VICTORY) {
		victory(world, score, window_flags, cam_transform, w, h);
	}
	else if (world.worldState == world.WORLD_DEFEAT) {
		defeat(world, score, window_flags, cam_transform, w, h);
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}
