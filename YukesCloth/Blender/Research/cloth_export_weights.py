import bpy
import json
import math
import bmesh

class ProxyEdge:
    def __init__(self):
        self.vertices = []
        
class ProxyVertex:
    def __init__(self,index,pos):
        self.index = index
        self.co = pos

class SkinVertex:
    def __init__(self, index, weight):
        self.index = index
        self.weight = weight
        
class EdgeLink:
    def __init__(self):
        self.vertices = []

def getFaceList( model ):
    faceList = []
    
    for f in model.data.polygons:
        faceTuple = []
        for idx in f.vertices:
            faceTuple.append(model.data.vertices[idx].index)
        
        faceList.append(  faceTuple )
        
    return faceList

def collapseModifierStack(obj):
    for modifier in obj.modifiers:
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    return

def GetModelDuplicate(source):
    new_obj = source.copy()
    new_obj.data = source.data.copy()
    bpy.context.collection.objects.link(new_obj)
    new_obj.name = "Temp_MODEL_LOW"
    
    # Apply all existing modifiers
    collapseModifierStack(new_obj)
    return new_obj

def DecimateModel(obj, ratio):
    # Decimate the model to a certain ratio
    modifier = obj.modifiers.new(name="Decimate", type='DECIMATE')
    modifier.ratio = ratio
    modifier.vertex_group = "Constraint Stretch"
    modifier.vertex_group_factor = 10

    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier="Decimate")
    return

def GetMeshEdgeVertices(obj):
    mesh = obj.data
    
    # Gets all model edges and their vertex coords
    model_edges = []
    for edge in mesh.edges:
        mesh_edge = ProxyEdge()
        
        for idx in edge.vertices:
            position = mesh.vertices[idx].co
            mesh_vertex = ProxyVertex(idx,position)
            mesh_edge.vertices.append(mesh_vertex)
            
        model_edges.append(mesh_edge)
    
    return model_edges

def find_nearest_vertex(mesh, target_coordinate):
    min_distance = float('inf')  # Initialize with infinity
    nearest_vertex_index = None
    
    for vertex_index, vertex in enumerate(mesh.vertices):
        # Calculate the distance between the target coordinate and the vertex
        distance = math.dist(target_coordinate,vertex.co)
        
        # Update the nearest vertex if this one is closer
        if distance < min_distance:
            min_distance = distance
            nearest_vertex_index = vertex_index
    
    return nearest_vertex_index

def GetCorrespondingVerts(source_obj, target_edges):
    source_edges = []
    source_mesh = source_obj.data
    
    for edge in target_edges:
        closest_edge = ProxyEdge()
        
        # Find closes matching vertices in source mesh
        for vert in edge.vertices:
            closest_vert_idx = find_nearest_vertex(source_mesh, vert.co)
            
            # Append closest vertex to edge indices
            source_vertex = source_mesh.vertices[closest_vert_idx]
            source_vertex = ProxyVertex(source_vertex.index,source_vertex.co)
            closest_edge.vertices.append(source_vertex)
        
        source_edges.append(closest_edge)
    return source_edges

def GetScaledEdgeConstraints(target_obj, ratio):
    
    # Duplicate the model and work on a lower res copy
    proxy_model = GetModelDuplicate(target_obj)
    DecimateModel(proxy_model,ratio)
    
    # Get all simplified edges 
    proxy_edges = GetMeshEdgeVertices(proxy_model)
    
    # DEBUG
    print("\n\nOperating on model: ", target_obj.name)
    print("Source Edge Count:",len(target_obj.data.edges))
    print("Operand Edge Count:",len(proxy_model.data.edges))
    
    # Find closest matching edges in source model using the proxy mesh
    target_edges = GetCorrespondingVerts(target_obj, proxy_edges)
    
    # Remove model from scene
    bpy.data.objects.remove(proxy_model)
    bpy.context.view_layer.objects.active = target_obj
    
    return target_edges
        
def GetGroupWeight(group_index, mesh_vert,obj):    
    for group in mesh_vert.groups:
         if (group.group == group_index):
             return group.weight
    return 0.0

def GetAllVertexWeights(group_name,obj):
    mesh = obj.data
    vertex_weights = [None] * len(mesh.vertices)
    group_index = obj.vertex_groups.find(group_name)
    
    for vert in mesh.vertices:
        weight = GetGroupWeight(group_index, vert, obj)
        vertex_weights[vert.index] = weight   
    return vertex_weights

def has_vertex_group(obj, group_name):
    for vertex_group in obj.vertex_groups:
        if vertex_group.name == group_name:
            return True
    
    return False

def GetEdgeWeights(obj,target):
    mesh = obj.data
    
    # Get all mesh blendweights for target group
    vertex_weights = GetAllVertexWeights(target, obj)
    edge_weights = [None] * len(mesh.edges)
    
    # Handle single vertex weight case
    if target == "Constraint Fixation":
        
        if (has_vertex_group(obj, "$Fixation")):
            vertex_weights = GetAllVertexWeights("$Fixation", obj)
        else:
            vertex_weights = GetAllVertexWeights("$SimRoot", obj)

        mesh_weights = []
        for vert in mesh.vertices:
            link = EdgeLink()
            skinVtx = SkinVertex(vert.index,vertex_weights[vert.index])
            link.vertices.append( skinVtx )
            mesh_weights.append(link)
            
        return mesh_weights
    
    # Loop through each edge in the mesh and collect weights
    for edge in mesh.edges:
        link = EdgeLink()
        for vertIdx in edge.vertices:
            skinVtx = SkinVertex(vertIdx,vertex_weights[vertIdx])
            link.vertices.append( skinVtx )
             
        edge_weights[edge.index] = link
    return edge_weights

def GetJson():
    # Initialize an empty dictionary
    json_data = {}
    return json_data

def SaveJsonToFile(jsonDoc, path):
    json_object = json.dumps(jsonDoc, indent=4)
    with open(path, "w") as outfile:
        outfile.write(json_object)
    outfile.close()
        
def SaveCTDataToJson(json,constraint,edge_links):
    constraint = constraint.replace(" ","-")
    json[constraint] = {"Size": len(edge_links)}
    json[constraint]["Links"] = {}
    json[constraint]["Links"]["Indices"] = {}
    json[constraint]["Links"]["Weights"] = {}
    
    link_idx = 0
    for link in edge_links:
        indices, weights = [], []
        for vert in link.vertices:
            indices.append(vert.index)
            weights.append(vert.weight)
        
        json[constraint]["Links"]["Indices"][str(link_idx)] = indices
            
        if (constraint == "Constraint-Fixation"):
            json[constraint]["Links"]["Indices"][str(link_idx)] = [indices[0],1.0]
            json[constraint]["Links"]["Weights"][str(link_idx)] = [weights[0],1.0,1.0]
            
        link_idx+=1
    return json
    
def SaveEdgeTableToJson(json,constraint,edge_links):
    constraint = constraint.replace(" ","-")
    json[constraint] = {"Size": len(edge_links)}
    json[constraint]["Links"] = {}
    json[constraint]["Links"]["Indices"] = {}
    json[constraint]["Links"]["Weights"] = {}
    
    link_idx = 0
    for link in edge_links:
        indices = [ link[0],link[1] ]
        delta = link[2]
        
        normal_value = 0.4950000047683716
        edge_constraint = [normal_value,normal_value,delta]
        
        if (constraint == "Constraint-Bend"):
            edge_constraint = [0,normal_value,delta]
        elif (constraint == "Constraint-Stretch"):
            edge_constraint = [1.0,delta,1.0]
            
        json[constraint]["Links"]["Indices"][str(link_idx)] = indices
        json[constraint]["Links"]["Weights"][str(link_idx)] = edge_constraint
        link_idx+=1
        
    return json

def getEdgeConstraints(obj):
    mesh = obj.data
    
    # Loop through each edge
    edge_data = []
    for edge in mesh.edges:
        indices = [ edge.vertices[1] , edge.vertices[0] ]
        delta = math.dist(mesh.vertices[indices[0]].co,
                          mesh.vertices[indices[1]].co)      
        indices.append(delta)
        edge_data.append(indices)
        
    return edge_data

def GetScaledEdgeTable(obj,scalar):
    # Map new edge list using a lower resolution model
    proxy_edges = GetScaledEdgeConstraints(obj, scalar)
    
    # Gather edge indices
    scaled_edges = []
    for edge in proxy_edges:
        indices = [ edge.vertices[0].index , edge.vertices[1].index ]
        delta = math.dist(edge.vertices[0].co,
                          edge.vertices[1].co)      
        indices.append(delta)
        scaled_edges.append(indices)

    return scaled_edges

def GetBlenderMode(obj):
    if obj:
        mode = bpy.context.object.mode
        return mode
    else:
        return None     
    
def SetBlenderMode(target_mode):
    bpy.ops.object.mode_set(mode=target_mode)
    return

def GetVertSelection(obj, use_single_select=True):
    # Ensure Blender is using object mode
    mode_source = GetBlenderMode(obj)
    SetBlenderMode('OBJECT')
    
    # Get the selected vertices
    selected_vertices = [v for v in obj.data.vertices if v.select]
    if len(selected_vertices) == 0:
        SetBlenderMode(mode_source)
        return None
    
    # return all indices if all are requested
    if (not use_single_select):
        verts = [v.index for v in selected_vertices]
        SetBlenderMode(mode_source)
        return verts
    
    # Revert scene status
    selected_vertex = selected_vertices[0].index
    SetBlenderMode(mode_source)
    return selected_vertex

def select_poly_island(mesh, index):
    SetBlenderMode('OBJECT')
    mesh.vertices[index].select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_mode(type="VERT")
    bpy.ops.mesh.select_linked(delimit={'SEAM'})

def select_vertex(obj,index, isolate_selection=True):
    if (isolate_selection):
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='DESELECT')

    SetBlenderMode('OBJECT')
    for vert in obj.data.vertices:
        vert.select = (vert.index == index)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_mode(type="VERT")
    return
           
def GetAllLinkedVertices(obj,vert_idx):
    mesh = obj.data
    select_poly_island(mesh,vert_idx)
    linked_verts = GetVertSelection(obj,use_single_select=False)
    return linked_verts

def vertex_coordinate_world(mesh, vertex_index):
    obj = bpy.context.object

    world_matrix = obj.matrix_world
    vertex = mesh.vertices[vertex_index]
    vertex_coord_obj = vertex.co

    vertex_coord_world = world_matrix @ vertex_coord_obj
    return vertex_coord_world

def GetNeighborVerts(mesh, vertex_index):
    # Initialize a list to store neighboring vertices' indices
    neighboring_vertex_indices = []
    bm = bmesh.from_edit_mesh(mesh)
    bm.verts.ensure_lookup_table()
    
    # Get the vertex
    vertex = bm.verts[vertex_index]
    
    # Iterate over the edges linked to the vertex
    for edge in vertex.link_edges:
        neighboring_vertex_indices.append(edge.other_vert(vertex).index)
    
    return neighboring_vertex_indices

def is_vertex_at_border(mesh, vertex_index):
    bm = bmesh.new()
    bm.from_mesh(mesh)
    
    # Get the vertex
    bm.verts.ensure_lookup_table()
    vertex = bm.verts[vertex_index]
    
    # Iterate over the edges linked to the vertex
    for edge in vertex.link_edges:
        # Check if the edge is only connected to one face
        if len(edge.link_faces) == 0:
            bm.free()
            return True
    
    # Free the bmesh
    bm.free()
    return False

def is_valid_delta_vert(mesh,query_vert, source_vert, index_range):
    
    query_coord = vertex_coordinate_world(mesh,query_vert.index)
    source_coord = vertex_coordinate_world(mesh,source_vert.index)
            
    is_distant = query_coord.z > source_coord.z
    is_within_range = (query_vert.index in index_range)
    
    return (is_distant and is_within_range)

def GetVerticalVertex(mesh, point_constraint, range_limit, seek_limit=-1):
    # Define how far we crawl
    if (seek_limit == -1):
        seek_limit = len(mesh.vertices)
        
    crawl_index = 0
    while crawl_index < seek_limit:
        # Define a list of all neighboring vertices
        source_index = point_constraint.index
        neighbor_verts = GetNeighborVerts(mesh, point_constraint.index)
        crawl_index += 1
        
        # Find Furthest Y coordinate in neighbors
        for index in neighbor_verts:
            near_vert = mesh.vertices[index]
            if is_valid_delta_vert(mesh,near_vert,point_constraint,range_limit):
                point_constraint = near_vert
        
        # If we cannot find any further coords, break the loop
        if source_index == point_constraint.index:
            return point_constraint
        
    return point_constraint

def FindFurthestVert_Vertical(mesh,source_vert_idx,vert_range,range=-1 ):
    # Define all vertices within our specified range
    target_vert =  mesh.vertices[source_vert_idx]
    
    # find most distant y coordinate within poly loop
    target_vert = GetVerticalVertex(mesh, target_vert, vert_range, range)
    index = target_vert.index
    return index

def GetVertexStretchLink(obj,vertex_index,range = -1):
     # Gets all vertices within range of the same polgonal island
    linked_verts = GetAllLinkedVertices(obj, vertex_index)
    
    # Finds the farthest coordinate on the same y axis
    target_vert = FindFurthestVert_Vertical(obj.data, vertex_index, linked_verts, range)
    return [target_vert, vertex_index]

def deselect_verts(source_mode):
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_mode(type="VERT")
    bpy.ops.mesh.select_all(action='DESELECT')

    SetBlenderMode('OBJECT')
    SetBlenderMode(source_mode)
    return

def GetAllMeshBindLinks(obj,range=-1):
    source_mode = GetBlenderMode(obj)
    mesh_vert_sel = [vert.index for vert in obj.data.vertices]
    mesh_links = []
    for vIdx in mesh_vert_sel:
        link = GetVertexStretchLink(obj, vIdx,range)
        delta = math.dist( obj.data.vertices[link[0]].co,
                           obj.data.vertices[link[1]].co )
        link.append(delta)
        mesh_links.append(link)

    deselect_verts(source_mode)
    return mesh_links

def get_vertex_groups_and_weights(obj, vertex_index):
    mesh = obj.data
    vertex_groups = mesh.vertices[vertex_index].groups
    bones_and_weights = {}
    bone_blacklist = {"Constraint Standard","Constraint Bend","Constraint Stretch","Constraint Fixation" }
    bone_pallette =  []

    for group in vertex_groups:
        bone_name = obj.vertex_groups[group.group].name
        weight = group.weight
        
        if bone_name not in bones_and_weights:
            if bone_name not in bone_blacklist and '$' not in bone_name:
                bones_and_weights[bone_name] = weight
                bone_pallette.append(bone_name)
    
    bones_and_weights["Total Groups"] = len(bone_pallette)
    bones_and_weights["Palette"] = bone_pallette
    return bones_and_weights

def getMeshColIndices(obj, mesh_cols):
    indices = []
    
    for verts in mesh_cols:
        for edge in verts:
            for index in edge:
                if index not in indices:
                    indices.append(index)
            
    return indices

def GetMeshColVerts( obj ):
    mode = GetBlenderMode(obj)
    SetBlenderMode('EDIT')
    
    mesh = obj.data
    mesh_cols = []
    
    num = 0
    for vert in mesh.vertices:
        edges = []
        neighbor_verts = GetNeighborVerts(mesh, vert.index)
        for neighbor in neighbor_verts:
            edges.append( [vert.index, neighbor] )
            num+=1
        mesh_cols.append(edges)
    
    indices = getMeshColIndices(obj, mesh_cols)
    
    SetBlenderMode(mode)
    return mesh_cols , indices

def UpdateMeshData(obj):
    SetBlenderMode('OBJECT')
    obj.data.update()
    return

def deselect_all_objects():
    SetBlenderMode('OBJECT')
    for sceneObj in bpy.context.scene.objects:
        sceneObj.select_set(False)
    return
    
def quadrify_mesh(obj):
    deselect_all_objects()
    
    bpy.context.view_layer.objects.active = obj
    SetBlenderMode('EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.tris_convert_to_quads()
    
    UpdateMeshData(obj)
    return

def get_face_indices(obj):
    mesh = obj.data
    faces = []
    for poly in mesh.polygons:
        faces.append( [v for v in poly.vertices] )
    return faces

def get_vertex_face_link_table(obj, face_indices):
    mesh = obj.data
    link_table = []
    
    for vert in mesh.vertices:
        vIdx = vert.index
        faces = []
        
        for i in range(len(face_indices)):
            face = face_indices[i]
            if vIdx in face:
                faces.append(i)
                
        # Store every known link for each specified vertex 
        link_table.append(faces)

    return link_table

def GetVertexFaceLinks(obj):
    mode = GetBlenderMode(obj)
    deselect_all_objects()
    
    # Build proxy object
    proxy_model = GetModelDuplicate(obj)
    quadrify_mesh(proxy_model)
    mesh_faces = get_face_indices(proxy_model)
    vtxFaceLinkTbl = get_vertex_face_link_table(obj, mesh_faces)

    # Remove model from scene
    bpy.data.objects.remove(proxy_model)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    SetBlenderMode(mode)
    return mesh_faces, vtxFaceLinkTbl

def GetMeshNormals(obj):
    modelNorms = list( range( len( obj.data.vertices ) ) )
    modelNorms = [ ( 0.0 , 0.0 , 1.0 ) for defaultNorm in modelNorms ]
    
    obj.data.calc_normals_split()
    for loop in obj.data.loops:
        normal_local = loop.normal.to_4d()
        normal_local.w = 0
        normal_local = (obj.matrix_world @ normal_local).to_3d()

        modelNorms[ loop.vertex_index ] = normal_local.to_tuple()

    return modelNorms

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

def Get_Mesh_Skin_Root_Verts(obj,json):
    mesh = obj.data
    is_sim_mesh = "S_" in mesh.name
    vertex_weights = GetAllVertexWeights("$SimRoot",obj)
    
    # Create SaveVert Entry
    save_indices = []
    paste_indicies = []
    
    # Get all indices
    for index in range( len(vertex_weights) ):
        weight = vertex_weights[index]
        if weight == 1.0:
            save_indices.append(index)
            paste_indicies.append(index)
            
        if not is_sim_mesh:
            save_indices.append(index)
    
    json["Mesh"]["SkinPaste"] = paste_indicies
    json["Mesh"]["SaveVerts"] = save_indices
    return json

def Get_Mesh_Skin(obj,json):
    mesh = obj.data
    json["Mesh"]["Skin"] = {}
    json["Mesh"]["Vertex"] = {}
    json["Mesh"]["Vertex Normal"] = {}
    vertex_normals = GetMeshNormals(obj)

    for vertice in mesh.vertices:
        vert_index = vertice.index
        vert_groups = get_vertex_groups_and_weights(obj,vert_index)
        json["Mesh"]["Skin"][str(vert_index)] = vert_groups
    
    for vertice in mesh.vertices:
        vert_index = vertice.index
        normal = vertex_normals[vert_index]
        coord = vertex_coordinate_world(mesh,vert_index)
        json["Mesh"]["Vertex"][str(vert_index)] = [coord.x,coord.z,coord.y]
        json["Mesh"]["Vertex Normal"][str(vert_index)] = [normal[0],-normal[2],normal[1]]
        
    return json

def Get_Mesh_Faces(obj,json):
    json["Mesh"]["SrcLink"] = {}
    face_list = getFaceList(obj)
    
    json["Mesh"]["Triangles"] = len(face_list)
    json["Mesh"]["SrcLink"] = face_list
    return json

def Get_Mesh_Force(obj,json):
    vertex_weights = GetAllVertexWeights("$SimRoot",obj)
    
    # Create gravity Force Entry
    vert_indices = []
    
    # Get all indices
    for index in range( len(vertex_weights) ):
        weight = vertex_weights[index]
        if weight != 1.0:
            vert_indices.append(index)

    json["Mesh"]["Force"] = vert_indices
    return json

def Get_All_Mesh_Col_Verts(obj,json):
    mesh_cols, indices = GetMeshColVerts(obj)
    json["Mesh"]["ColVerts"] = {}
    
    for i in range( len(mesh_cols) ):
        edge_cols = mesh_cols[i]
        edge_indices = []
        
        for edge in edge_cols:
            for index in edge:
                edge_indices.append(index)
        
        json["Mesh"]["ColVerts"][str(i)] = edge_indices
        
    json["Mesh"]["ColVerts"]["Palette"] = indices
    return json

def Get_Mesh_RCN(obj,json):
    faces, links = GetVertexFaceLinks(obj)
    json["Mesh"]["RCN"] = {}
    json["Mesh"]["RCN"]["Faces"] = {"Size": len(faces)}
    json["Mesh"]["RCN"]["Links"] = {}
    
    for i in range(len(faces)):
        faceDef = faces[i]
        json["Mesh"]["RCN"]["Faces"][str(i)] = faceDef

    for i in range(len(links)):
        face_link = links[i]
        json["Mesh"]["RCN"]["Links"][str(i)] = face_link

    return json

def Get_Mesh_Json(obj, json):
    mesh = obj.data

    subobj_verts = [vert.index for vert in mesh.vertices]
    json["Mesh"] = {}
    json["Mesh"]["SubObj Verts"] = subobj_verts
    json["Mesh"]["Sim Verts"] = subobj_verts
    json["Mesh"]["SkinCalc"] = subobj_verts

    json = Get_Mesh_Skin(obj,json) # Add Skin
    json = Get_Mesh_Skin_Root_Verts(obj,json) # Add SkinPaste+OldVtxSave data
    json = Get_Mesh_Faces(obj,json) # Add triangles (SrcLink)
    json = Get_Mesh_Force(obj,json) # Add Force
    json = Get_All_Mesh_Col_Verts(obj,json) # Add ColVerts
    json = Get_Mesh_RCN(obj,json) # Add Recalc normals
    return json

def main():
    generate_mesh_skin = True
    active_obj  = bpy.context.object
    jsonDoc = GetJson()
    save_constraints = ["Standard","Bend","Stretch","Fixation"]
    save_constraints = ["Standard","Bend","Stretch","Fixation"]
    output = []
    
    # Creates sim weight paint groups
    if (generate_mesh_skin):
        generate_sim_root_skin(active_obj)

    for constraint in save_constraints:
        if (constraint == "Standard"):
            edge_data = getEdgeConstraints(active_obj)
            jsonDoc = SaveEdgeTableToJson(jsonDoc, "Constraint " + constraint, edge_data)
            output.append(constraint)
        elif (constraint == "Bend"):
            edge_data = GetAllMeshBindLinks(active_obj,range=5)
            jsonDoc = SaveEdgeTableToJson(jsonDoc, "Constraint " + constraint, edge_data)
            output.append(constraint)
        elif (constraint == "Stretch"):
            edge_data = GetAllMeshBindLinks(active_obj)
            jsonDoc = SaveEdgeTableToJson(jsonDoc, "Constraint " + constraint, edge_data)
            output.append(constraint)
        else:
            edge_data = GetEdgeWeights(active_obj,  "Constraint " + constraint)
            jsonDoc = SaveCTDataToJson(jsonDoc, "Constraint " + constraint, edge_data)
            output.append(constraint)
    
    # Append mesh data
    jsonDoc = Get_Mesh_Json(active_obj,jsonDoc)

    # Save data to json
    jsonDoc["Model"]    = {"SubObj Name" : active_obj.data.name, 
                           "Sim Verts" : len(active_obj.data.vertices),
                           "Constraints" : output }
    SaveJsonToFile(jsonDoc,"C:/Users/wauke/Desktop/new_weights.json")
    print("All mesh sim data saved to file.")
    
if __name__ == "__main__":
    main()