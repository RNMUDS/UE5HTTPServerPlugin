#!/usr/bin/env python3
"""UE5 MCP Server for Claude Code - Enhanced with batch operations"""

import asyncio
import httpx
import logging
from typing import Any, List, Dict
from mcp.server import Server, NotificationOptions
from mcp.server.models import InitializationOptions
import mcp.server.stdio
import mcp.types as types

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

UE5_BASE_URL = "http://localhost:8080"

server = Server("ue5-control")

@server.list_tools()
async def list_tools() -> list[types.Tool]:
    return [
        types.Tool(
            name="create_actor",
            description="Create a new actor in UE5",
            inputSchema={
                "type": "object",
                "properties": {
                    "type": {
                        "type": "string",
                        "enum": ["Cube", "Sphere", "Cylinder", "Plane", "Light", "Camera"],
                        "description": "Type of actor"
                    },
                    "name": {"type": "string", "description": "Actor name"},
                    "location": {
                        "type": "object",
                        "properties": {
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        },
                        "required": ["x", "y", "z"]
                    },
                    "material": {
                        "type": "object",
                        "properties": {
                            "type": {"type": "string", "enum": ["color", "preset"]},
                            "color": {
                                "type": "object",
                                "properties": {
                                    "r": {"type": "number"},
                                    "g": {"type": "number"},
                                    "b": {"type": "number"},
                                    "a": {"type": "number"}
                                }
                            },
                            "name": {"type": "string"}
                        }
                    },
                    "scale": {
                        "type": "object",
                        "properties": {
                            "uniform": {"type": "number", "description": "Uniform scale for all axes"},
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        }
                    },
                    "dimensions": {
                        "type": "object",
                        "properties": {
                            "width": {"type": "number", "description": "Width in UE units"},
                            "depth": {"type": "number", "description": "Depth in UE units"},
                            "height": {"type": "number", "description": "Height in UE units"}
                        },
                        "description": "Physical dimensions in UE units (overrides scale)"
                    },
                    "intensity": {"type": "number", "description": "Light intensity (for Light actors only)"},
                    "attenuationRadius": {"type": "number", "description": "Light attenuation radius (for Light actors only)"}
                },
                "required": ["type", "name", "location"]
            }
        ),
        types.Tool(
            name="create_actors_batch",
            description="Create multiple actors at once in UE5",
            inputSchema={
                "type": "object",
                "properties": {
                    "actors": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "type": {
                                    "type": "string",
                                    "enum": ["Cube", "Sphere", "Cylinder", "Plane", "Light", "Camera"]
                                },
                                "name": {"type": "string"},
                                "location": {
                                    "type": "object",
                                    "properties": {
                                        "x": {"type": "number"},
                                        "y": {"type": "number"},
                                        "z": {"type": "number"}
                                    },
                                    "required": ["x", "y", "z"]
                                },
                                "color": {
                                    "type": "object",
                                    "properties": {
                                        "r": {"type": "number"},
                                        "g": {"type": "number"},
                                        "b": {"type": "number"},
                                        "a": {"type": "number"}
                                    }
                                },
                                "scale": {
                                    "type": "object",
                                    "properties": {
                                        "uniform": {"type": "number"},
                                        "x": {"type": "number"},
                                        "y": {"type": "number"},
                                        "z": {"type": "number"}
                                    }
                                },
                                "dimensions": {
                                    "type": "object",
                                    "properties": {
                                        "width": {"type": "number"},
                                        "depth": {"type": "number"},
                                        "height": {"type": "number"}
                                    }
                                }
                            },
                            "required": ["type", "name", "location"]
                        },
                        "description": "Array of actors to create"
                    }
                },
                "required": ["actors"]
            }
        ),
        types.Tool(
            name="create_grid",
            description="Create a grid of actors",
            inputSchema={
                "type": "object",
                "properties": {
                    "type": {
                        "type": "string",
                        "enum": ["Cube", "Sphere", "Cylinder", "Plane"],
                        "description": "Type of actors to create"
                    },
                    "base_name": {"type": "string", "description": "Base name for actors (will append _X_Y)"},
                    "rows": {"type": "integer", "minimum": 1, "description": "Number of rows"},
                    "columns": {"type": "integer", "minimum": 1, "description": "Number of columns"},
                    "spacing": {"type": "number", "description": "Distance between actors"},
                    "start_location": {
                        "type": "object",
                        "properties": {
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        },
                        "required": ["x", "y", "z"]
                    },
                    "color": {
                        "type": "object",
                        "properties": {
                            "r": {"type": "number"},
                            "g": {"type": "number"},
                            "b": {"type": "number"},
                            "a": {"type": "number"}
                        }
                    },
                    "scale": {
                        "type": "object",
                        "properties": {
                            "uniform": {"type": "number"},
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        }
                    },
                    "dimensions": {
                        "type": "object",
                        "properties": {
                            "width": {"type": "number"},
                            "depth": {"type": "number"},
                            "height": {"type": "number"}
                        }
                    }
                },
                "required": ["type", "base_name", "rows", "columns", "spacing", "start_location"]
            }
        ),
        types.Tool(
            name="move_actor",
            description="Move an actor in UE5",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {"type": "string"},
                    "location": {
                        "type": "object",
                        "properties": {
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        }
                    }
                },
                "required": ["actor_name", "location"]
            }
        ),
        types.Tool(
            name="get_scene",
            description="Get UE5 scene information",
            inputSchema={"type": "object", "properties": {}}
        ),
        types.Tool(
            name="set_actor_color",
            description="Set the color of an actor",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {"type": "string"},
                    "color": {
                        "type": "object",
                        "properties": {
                            "r": {"type": "number"},
                            "g": {"type": "number"},
                            "b": {"type": "number"},
                            "a": {"type": "number"}
                        },
                        "required": ["r", "g", "b"]
                    }
                },
                "required": ["actor_name", "color"]
            }
        ),
        types.Tool(
            name="set_actor_scale",
            description="Set the scale of an actor",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {"type": "string"},
                    "scale": {
                        "type": "object",
                        "properties": {
                            "uniform": {"type": "number"},
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"}
                        }
                    }
                },
                "required": ["actor_name", "scale"]
            }
        ),
        types.Tool(
            name="rotate_actor",
            description="Rotate an actor",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {"type": "string"},
                    "rotation": {
                        "type": "object",
                        "properties": {
                            "pitch": {"type": "number"},
                            "yaw": {"type": "number"},
                            "roll": {"type": "number"}
                        },
                        "required": ["pitch", "yaw", "roll"]
                    }
                },
                "required": ["actor_name", "rotation"]
            }
        ),
        types.Tool(
            name="delete_actor",
            description="Delete an actor from the scene",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {"type": "string"}
                },
                "required": ["actor_name"]
            }
        ),
        types.Tool(
            name="delete_all_actors",
            description="Delete all user-created actors from the scene",
            inputSchema={
                "type": "object",
                "properties": {}
            }
        )
    ]

@server.call_tool()
async def call_tool(name: str, arguments: dict[str, Any]) -> list[types.TextContent]:
    logger.info(f"Tool called: {name} with args: {arguments}")

    try:
        async with httpx.AsyncClient(timeout=10.0) as client:
            # ヘルスチェック
            try:
                health = await client.get(f"{UE5_BASE_URL}/health")
                if health.status_code != 200:
                    return [types.TextContent(
                        type="text",
                        text="❌ UE5 HTTP server not responding. Start PIE mode in UE5."
                    )]
            except:
                return [types.TextContent(
                    type="text",
                    text="❌ Cannot connect to UE5. Ensure UE5 is running with HTTP server on port 8080."
                )]

            if name == "create_actor":
                # MCPのmaterialパラメータをUE5のcolorパラメータに変換
                ue5_params = {
                    "type": arguments["type"],
                    "name": arguments["name"],
                    "location": arguments["location"]
                }
                
                # materialパラメータが存在し、colorタイプの場合
                if "material" in arguments and arguments["material"].get("type") == "color":
                    ue5_params["color"] = arguments["material"]["color"]
                
                # scaleパラメータ
                if "scale" in arguments:
                    ue5_params["scale"] = arguments["scale"]
                
                # dimensionsパラメータ
                if "dimensions" in arguments:
                    ue5_params["dimensions"] = arguments["dimensions"]
                
                # Light特有のパラメータ
                if arguments["type"] == "Light":
                    if "intensity" in arguments:
                        ue5_params["intensity"] = arguments["intensity"]
                    if "attenuationRadius" in arguments:
                        ue5_params["attenuationRadius"] = arguments["attenuationRadius"]
                
                response = await client.post(f"{UE5_BASE_URL}/actors", json=ue5_params)
                if response.status_code == 200:
                    result = response.json()
                    info_parts = [f"✅ Created {arguments['type']} '{arguments['name']}' at ({arguments['location']['x']}, {arguments['location']['y']}, {arguments['location']['z']})"]
                    
                    if "color" in ue5_params:
                        c = ue5_params["color"]
                        info_parts.append(f"color: ({c.get('r', 1):.1f}, {c.get('g', 1):.1f}, {c.get('b', 1):.1f})")
                    
                    if "dimensions" in ue5_params:
                        d = ue5_params["dimensions"]
                        info_parts.append(f"dimensions: {d.get('width')}x{d.get('depth')}x{d.get('height')}")
                    elif "scale" in ue5_params:
                        s = ue5_params["scale"]
                        if "uniform" in s:
                            info_parts.append(f"scale: {s['uniform']}")
                        else:
                            info_parts.append(f"scale: ({s.get('x', 1)}, {s.get('y', 1)}, {s.get('z', 1)})")
                    
                    return [types.TextContent(
                        type="text",
                        text=" | ".join(info_parts)
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]

            elif name == "create_actors_batch":
                response = await client.post(f"{UE5_BASE_URL}/actors/batch", json=arguments)
                if response.status_code == 200:
                    result = response.json()
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Batch creation complete: {result.get('created', 0)} actors created, {result.get('failed', 0)} failed"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]

            elif name == "create_grid":
                # グリッド作成のためのアクター配列を生成
                actors = []
                for row in range(arguments["rows"]):
                    for col in range(arguments["columns"]):
                        actor_data = {
                            "type": arguments["type"],
                            "name": f"{arguments['base_name']}_{row}_{col}",
                            "location": {
                                "x": arguments["start_location"]["x"] + col * arguments["spacing"],
                                "y": arguments["start_location"]["y"] + row * arguments["spacing"],
                                "z": arguments["start_location"]["z"]
                            }
                        }
                        
                        if "color" in arguments:
                            actor_data["color"] = arguments["color"]
                        
                        if "scale" in arguments:
                            actor_data["scale"] = arguments["scale"]
                        
                        if "dimensions" in arguments:
                            actor_data["dimensions"] = arguments["dimensions"]
                        
                        actors.append(actor_data)
                
                batch_data = {"actors": actors}
                response = await client.post(f"{UE5_BASE_URL}/actors/batch", json=batch_data)
                
                if response.status_code == 200:
                    result = response.json()
                    total = arguments["rows"] * arguments["columns"]
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Created {arguments['rows']}x{arguments['columns']} grid of {arguments['type']}s | Total: {total} actors | Spacing: {arguments['spacing']} units"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]

            elif name == "move_actor":
                response = await client.put(
                    f"{UE5_BASE_URL}/actors/{arguments['actor_name']}/location",
                    json={"location": arguments["location"]}
                )
                if response.status_code == 200:
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Moved '{arguments['actor_name']}' to ({arguments['location']['x']}, {arguments['location']['y']}, {arguments['location']['z']})"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]

            elif name == "get_scene":
                response = await client.get(f"{UE5_BASE_URL}/scene")
                if response.status_code == 200:
                    data = response.json()
                    actors = data.get("actors", [])
                    text = f"Scene has {data.get('actorCount', 0)} actors:\n"
                    for actor in actors[:10]:  # 最初の10個まで
                        text += f"- {actor['name']} at ({actor['location']['x']:.0f}, {actor['location']['y']:.0f}, {actor['location']['z']:.0f})\n"
                    if len(actors) > 10:
                        text += f"... and {len(actors) - 10} more actors"
                    return [types.TextContent(type="text", text=text)]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]
            
            elif name == "set_actor_color":
                response = await client.put(
                    f"{UE5_BASE_URL}/actors/{arguments['actor_name']}/color",
                    json={"color": arguments["color"]}
                )
                if response.status_code == 200:
                    c = arguments["color"]
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Changed color of '{arguments['actor_name']}' to ({c['r']:.1f}, {c['g']:.1f}, {c['b']:.1f})"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]
            
            elif name == "set_actor_scale":
                response = await client.put(
                    f"{UE5_BASE_URL}/actors/{arguments['actor_name']}/scale",
                    json={"scale": arguments["scale"]}
                )
                if response.status_code == 200:
                    scale = arguments["scale"]
                    if "uniform" in scale:
                        scale_text = f"uniform scale {scale['uniform']}"
                    else:
                        scale_text = f"scale ({scale.get('x', 1)}, {scale.get('y', 1)}, {scale.get('z', 1)})"
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Set {scale_text} for '{arguments['actor_name']}'"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]
            
            elif name == "rotate_actor":
                response = await client.put(
                    f"{UE5_BASE_URL}/actors/{arguments['actor_name']}/rotation",
                    json={"rotation": arguments["rotation"]}
                )
                if response.status_code == 200:
                    r = arguments["rotation"]
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Rotated '{arguments['actor_name']}' to (pitch: {r['pitch']}°, yaw: {r['yaw']}°, roll: {r['roll']}°)"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]
            
            elif name == "delete_actor":
                response = await client.delete(
                    f"{UE5_BASE_URL}/actors/{arguments['actor_name']}"
                )
                if response.status_code == 200:
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Deleted actor '{arguments['actor_name']}'"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]
            
            elif name == "delete_all_actors":
                response = await client.delete(f"{UE5_BASE_URL}/actors")
                if response.status_code == 200:
                    result = response.json()
                    return [types.TextContent(
                        type="text",
                        text=f"✅ Deleted {result.get('deletedCount', 0)} actors from the scene"
                    )]
                else:
                    return [types.TextContent(type="text", text=f"❌ Failed: {response.text}")]

    except Exception as e:
        logger.error(f"Error: {e}")
        return [types.TextContent(type="text", text=f"❌ Error: {str(e)}")]

async def main():
    async with mcp.server.stdio.stdio_server() as (read_stream, write_stream):
        await server.run(
            read_stream,
            write_stream,
            InitializationOptions(
                server_name="ue5-control",
                server_version="0.2.0",
                capabilities=server.get_capabilities(
                    notification_options=NotificationOptions(),
                    experimental_capabilities={}
                )
            )
        )

if __name__ == "__main__":
    asyncio.run(main())