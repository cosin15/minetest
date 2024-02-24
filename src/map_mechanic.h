/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#pragma once

#include <memory>
#include <vector>
#include <map>

#include "irrlichttypes_bloated.h"
#include "util/container.h"

class IGameDef;
class Map;
class NodeDefManager;
class MapBlock;
class ServerEnvironment;
struct MapMechanicDeps;


class MapMechanic {

	public:
	MapMechanic(IGameDef *gamedef, const NodeDefManager *nodedef, Map *map)
	: m_gamedef(gamedef), m_nodedef(nodedef), m_map(map)
	{}

	inline void push_node(const v3s16& node) {
		m_queue.push_back(node);
	}

	virtual void run(std::map<v3s16, MapBlock*>& modified_blocks,
			MapMechanicDeps& deps) = 0;

	virtual ~MapMechanic() {}

	inline UniqueQueue<v3s16> *getQueue() {
		return &m_queue;
	}

	protected:
	UniqueQueue<v3s16> m_queue;

	IGameDef *m_gamedef;
	const NodeDefManager *m_nodedef;
	Map *m_map;
};

