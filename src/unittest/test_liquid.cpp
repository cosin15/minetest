/*
Minetest

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "test.h"

#include "dummymap.h"
#include "gamedef.h"
#include "liquid_system.h"
#include "map_mechanic_events.h"
//#include "mock_server.h"
#include "serverenvironment.h"
#include "server.h"


class TestLiquid : public TestBase
{
	public:
		TestLiquid() { TestManager::registerTestModule(this); }
		const char *getName() { return "TestLiquid"; };

		void runTests(IGameDef *gamedef);

		void testLiquid(IGameDef *gamedef);

	private:
		struct ContentIds {
			content_t water_source;
			content_t water_flowing;
			content_t dirt;
		};

		void fill_map(Map *map, v3s16 from, v3s16 to, MapNode n);

		MapNode create_node(struct ContentIds& ids, const char *def);

};

static TestLiquid g_test_instance;


MapNode TestLiquid::create_node(struct ContentIds& ids, const char *def)
{
	MapNode n{};

	switch(def[0]) {
		case ' ': n.setContent(CONTENT_AIR      ); break;
		case 'w': n.setContent(ids.water_flowing); break;
		case 'W': n.setContent(ids.water_source ); break;
		case 'D': n.setContent(ids.dirt         ); break;
		default : n.setContent(CONTENT_IGNORE   ); break;
	}

	switch(def[1]) {
		case '0': n.setParam2(0); break;
		case '1': n.setParam2(1); break;
		case '2': n.setParam2(2); break;
		case '3': n.setParam2(3); break;
		case '4': n.setParam2(4); break;
		case '5': n.setParam2(5); break;
		case '6': n.setParam2(6); break;
		case '7': n.setParam2(7); break;
		case '8': n.setParam2(8); break;
		case '9': n.setParam2(9); break;
		default : n.setParam2(0); break;
	}

	return n;
}

void TestLiquid::fill_map(Map *map, v3s16 from, v3s16 to, MapNode n)
{
	for(s16 x = from.X; x < to.X; ++x) {
		for(s16 y = from.Y; y < to.Y; ++y) {
			for(s16 z = from.Z; z < to.Z; ++z) {
				map->setNode({x,y,z}, n);
			}
		}
	}
}


void TestLiquid::runTests(IGameDef *gamedef)
{
	TEST(testLiquid, gamedef);
}

void TestLiquid::testLiquid(IGameDef *gamedef)
{
	auto map = new DummyMap(gamedef, {-2,-2,-2}, {2, 2, 2});
	auto ndef = (NodeDefManager *)map->getNodeDefManager();

	ContentFeatures water_source_def;
	water_source_def.name = "water_source";
	water_source_def.liquid_type = LIQUID_SOURCE;
	water_source_def.liquid_renewable = true;
	water_source_def.liquid_viscosity = 7;
	water_source_def.liquid_range = 7;
	water_source_def.liquid_alternative_flowing = "water_flowing";

	ContentFeatures water_flowing_def;
	water_flowing_def.name = "water_flowing";
	water_flowing_def.liquid_type = LIQUID_FLOWING;
	water_flowing_def.liquid_renewable = true;
	water_flowing_def.liquid_viscosity = 7;
	water_flowing_def.liquid_range = 7;
	water_flowing_def.liquid_alternative_flowing = "water_source";

	ContentFeatures dirt_def;
	dirt_def.name = "dirt";

	ContentIds ids = {
		.water_source = ndef->set(water_source_def.name, water_source_def),
		.water_flowing = ndef->set(water_flowing_def.name, water_flowing_def),
		.dirt = ndef->set(dirt_def.name, dirt_def),
	};

	fill_map(map, {-32,-32,-32}, {32,32,32}, MapNode(CONTENT_AIR));
	fill_map(map, {-32,-32,-32}, {32, 7,32}, MapNode(ids.dirt));

	auto liquid_system = createLiquidSystem(
		gamedef,
		map->getNodeDefManager(),
		map
	);

	std::map<v3s16, MapBlock*> modified_blocks;


	ndef->resolveCrossrefs();


	MapNode node = map->getNode({0,10,0});
	UASSERT(node.getContent() == CONTENT_AIR);

	map->setNode({0,10,0}, MapNode(ids.water_source));
	node = map->getNode({0,10,0});
	UASSERT(node.getContent() == ids.water_source);
	liquid_system->push_node({0,10,0});

	std::vector<v3s16> check_for_falling;


	int calls_node_on_flood = 0;
	int calls_on_liquid_transformed = 0;
	int calls_check_for_falling = 0;

	MapMechanicDeps deps = {
		.node_on_flood         = [&] (auto p, auto node, auto newnode) -> bool {
			++calls_node_on_flood;
			return true;
		},
		.on_liquid_transformed = [&] (auto list) {
			//UASSERT(list.size() == 1lu);
			++calls_on_liquid_transformed;
		},
		.check_for_falling     = [&] (auto n) {
			check_for_falling.push_back(n);
			++calls_check_for_falling;
		},
	};

	liquid_system->run(modified_blocks, deps);
	liquid_system->run(modified_blocks, deps);

	//UASSERT(calls_node_on_flood == 0u);
	UASSERT(calls_on_liquid_transformed == 2u);
	//UASSERT(calls_check_for_falling == 0u);
	//UASSERT(modified_blocks.size() == 1lu);
	//UASSERT(liquid_system->getQueue()->size() == 0lu);

	node = map->getNode({0,+10,0});
	UASSERT(node.getContent() == ids.water_source);

	node = map->getNode({0,+9,0});
	UASSERT(node.getContent() == ids.water_flowing);

	node = map->getNode({0,+8,0});
	UASSERT(node.getContent() == CONTENT_AIR);

	calls_on_liquid_transformed = 0u;
	liquid_system->run(modified_blocks, deps);
	UASSERT(calls_on_liquid_transformed == 1u);

	node = map->getNode({0,+8,0});
	//UASSERT(node.getContent() == ids.water_flowing);
	UASSERT(node.getContent() == ids.water_source); // Unexpected !!

	node = map->getNode({0,+7,0});
	UASSERT(node.getContent() == CONTENT_AIR);


	calls_on_liquid_transformed = 0u;
	liquid_system->run(modified_blocks, deps);
	UASSERT(calls_on_liquid_transformed == 1u);

	node = map->getNode({0,+7,0});
	UASSERT(node.getContent() == ids.water_flowing);

	node = map->getNode({0,+6,0});
	UASSERT(node.getContent() == ids.dirt);

	node = map->getNode({-1,+7,+0});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+1,+7,+0});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+0,+7,-1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+0,+7,+1});
	UASSERT(node.getContent() == CONTENT_AIR);


	calls_on_liquid_transformed = 0u;
	liquid_system->run(modified_blocks, deps);
	UASSERT(calls_on_liquid_transformed == 1u);

	node = map->getNode({0,+7,0});
	UASSERT(node.getContent() == ids.water_flowing);

	node = map->getNode({-1,+7,+0});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+1,+7,+0});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+0,+7,-1});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+0,+7,+1});
	UASSERT(node.getContent() == ids.water_source); // WTF

	node = map->getNode({-1,+7,+1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+1,+7,+1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({-1,+7,-1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+1,+7,-1});
	UASSERT(node.getContent() == CONTENT_AIR);



	calls_on_liquid_transformed = 0u;
	liquid_system->run(modified_blocks, deps);
	UASSERT(calls_on_liquid_transformed == 1u);

	node = map->getNode({-1,+7,+0});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+1,+7,+0});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+0,+7,-1});
	UASSERT(node.getContent() == ids.water_source); // WTF
	node = map->getNode({+0,+7,+1});
	UASSERT(node.getContent() == ids.water_source); // WTF

	node = map->getNode({-1,+7,+1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+1,+7,+1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({-1,+7,-1});
	UASSERT(node.getContent() == CONTENT_AIR);
	node = map->getNode({+1,+7,-1});
	UASSERT(node.getContent() == CONTENT_AIR);

}


