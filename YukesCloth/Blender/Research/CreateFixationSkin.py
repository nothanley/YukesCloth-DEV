import bpy
from collections import defaultdict

def GetMeshParentObj(mesh):
    parent_obj = None
    for obj in bpy.context.scene.objects:
        if mesh == obj.data:
            parent_obj = obj
            break
    return parent_obj

def AddSkinVertex(mesh, vertex_index, weight, group_title):
    parent_obj = GetMeshParentObj(mesh)
    if not parent_obj: return

    mesh_vertex = mesh.vertices[vertex_index]
    vertex_group = parent_obj.vertex_groups.get(group_title)
    
    if not vertex_group:
        vertex_group = parent_obj.vertex_groups.new(name=group_title)

    vertex_group.add([vertex_index], weight, 'REPLACE')
    return

def get_vertex_islands(obj):
    # Duplicate the original mesh
    source_obj = obj
    bpy.ops.object.duplicate()
    duplicated_obj = bpy.context.active_object
    mesh_verts = [vert.co for vert in source_obj.data.vertices]

    
    # Switch to object mode
    bpy.context.view_layer.objects.active = duplicated_obj
    duplicated_obj.select_set(True)
    
    # Separate each island into a separate object
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.separate(type='LOOSE')
    
    # Get the newly created island objects
    bpy.ops.object.mode_set(mode='OBJECT')
    island_objects = [obj for obj in bpy.context.selected_objects if obj.type == 'MESH']
    
    # Initialize a list to store vertex coordinates for each island
    island_vertices = []
    
    # Collect vertex coordinates for each island
    for island_obj in island_objects:
        island_mesh = island_obj.data
        island_vertex_coords = []
        for vertex in island_mesh.vertices:
            island_vertex_coords.append(vertex.co.copy())  # Make a copy of the vertex coordinate
        island_vertices.append(island_vertex_coords)

    # Island stuff...
    mesh_islands = []
    for island_vertex_coords in island_vertices:
        island_indices = []
        for coord in island_vertex_coords:
            index = mesh_verts.index(coord)
            island_indices.append(index)
        mesh_islands.append(island_indices)
    
    # Clean up: Delete the island objects
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.object.select_all(action='DESELECT')
    for island_obj in island_objects:
        island_obj.select_set(True)
    bpy.ops.object.delete()
    
    bpy.context.view_layer.objects.active = source_obj
    return mesh_islands

def get_island_weights(obj, island_indices, index, threshold_percent=0.1):
    # Get the mesh data
    mesh = obj.data
    
    # Get the world matrix of the object
    world_matrix = obj.matrix_world
    
    # Transform vertices to world coordinates and collect Z coordinates
    world_coords_z = []
    for vertex in mesh.vertices:
        if (vertex.index in island_indices):
            vertex_co_world = world_matrix @ vertex.co
            world_coords_z.append(vertex_co_world.z)
    
    # Get the highest and lowest Z coordinates in world space
    max_z = max(world_coords_z)
    min_z = min(world_coords_z)
    
    # Calculate the range of Z coordinates
    z_range = max_z - min_z
    
    # Calculate the threshold value based on the percentage of the Z range
    threshold = min_z + (threshold_percent * (max_z - min_z))
    
    # Iterate over the vertices and set the weights based on Z coordinates
    for vertex in mesh.vertices:
        if (vertex.index in island_indices):
            # Transform the vertex to world coordinates
            vertex_co_world = world_matrix @ vertex.co
            
            # Calculate the weight based on the Z coordinate and apply clamping
            if vertex_co_world.z > threshold:
                AddSkinVertex(mesh, vertex.index, 1.0, "$SimRoot")
    return

def generate_sim_root_skin(obj):
    source_mode = bpy.context.object.mode
    bpy.ops.object.mode_set(mode='OBJECT')
    
    mesh_islands = get_vertex_islands(obj)
    
    for i in range(len(mesh_islands)):        
        get_island_weights(obj,mesh_islands[i],i,0.9)
    
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode=source_mode)
    return

# Example usage:
obj = bpy.context.object
generate_sim_root_skin(obj)