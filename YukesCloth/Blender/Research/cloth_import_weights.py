import bpy
import os
import json

def GetJsonFile(filePath):
    f = open(filePath)
    return  json.load(f)
 
def GetCurrentDir():
    dir = os.path.dirname(os.path.realpath(__file__))
    path, filename = os.path.split(dir)
    return  path

def GetYclJson():
    path = GetCurrentDir() + "\\out_weights.json"
    user_json = GetJsonFile(path)
    return  user_json

def GetSceneObjs():
    scene = bpy.context.scene
    objects = scene.objects
    object_list = [obj for obj in objects if obj.type == 'MESH']
    return  object_list

def FilterObjectsForTarget(objects, target_name, target_verts):
    filtered_objs = []

    for obj in objects:
        mesh = obj.data
        vertex_count = len(mesh.vertices)
        if (target_name == mesh.name):
            filtered_objs.append(mesh)
            
    return filtered_objs

def GetAllSceneMeshTargets(model_name,model_verts,model_constraints):
    scene_objects = GetSceneObjs()
    scene_objects = FilterObjectsForTarget(scene_objects, model_name, model_verts)
    
    print("\n\nTarget Model : ", model_name , "Target Verts: ",  model_verts)
    print("Target Constraints : ", model_constraints)
    print("Found target models: " , scene_objects)
    return scene_objects

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

def NormalizeVector(vector):
    vector = [float(i)/max(vector) for i in vector]
    return vector

def MapConstraintToMesh(constraint, mesh, size, title):
    for x in range(size):
        weights = constraint["Links"]["Weights"][str(x)]
        indices = constraint["Links"]["Indices"][str(x)]
        
        if (title == "Stretch"):
            # Normalize weight values
            weights[0] = weights[1]
            indices[0] = indices[1]
            
            weights[0] = float(weights[0] * .01)
            weights[1] = float(weights[1] * .01)
        elif (title == "Fixation"):
            # Map only to first index
            indices[1] = indices[0]
            weights[1] = weights[0]
            
        # Append skin to vertex groups
        AddSkinVertex(mesh, indices[0],weights[0], "$"+title)
        AddSkinVertex(mesh, indices[1],weights[1], "$"+title)
    return

def InitConstraint(mesh, cnstr_type, json):
    constraint_dataset = cnstr_type
    
    try:
        constraint_dataset = json["Constraint-"+cnstr_type]
        print("\n\nMapping constraint for type: " , cnstr_type)
    except:
        print("\nFailed to map constraint for type: " , cnstr_type)
        return
        
    total_cnstr_size = constraint_dataset["Size"]
    print("Constraint Type: ", cnstr_type)
    print("Affected Verts: ", total_cnstr_size)
    MapConstraintToMesh(constraint_dataset, mesh, total_cnstr_size, cnstr_type)
    return

class SimMesh:
    def __init__(self, model_name, model_json, obj):
        self.name = model_name
        self.json = model_json
        self.constraints = self.json["Model"]["Constraints"]
        self.scene_obj = obj

    def map_constraints(self):
        for type in self.constraints:
            if (type == "Stretch" or type == "Fixation"):
                InitConstraint(self.scene_obj,type,self.json)
        return
        
        
def main():

    # Get user ycl json and a list of all current scene meshes
    user_json = GetYclJson()
    model_constraints = user_json["Model"]["Constraints"]
    
    # Load target mesh data
    model_name = user_json["Model"]["SubObj Name"]
    model_verts = user_json["Model"]["Sim Verts"]

    # Get all target meshes
    scene_objects = GetAllSceneMeshTargets(model_name,model_verts,model_constraints)
    
    if (len(scene_objects) == 0 or len(model_constraints) == 0):
        return
    
    # Iterate through Target Meshes
    for object in scene_objects:
        sim_model = SimMesh(model_name,user_json,object)
        sim_model.map_constraints()


if __name__ == "__main__":
    main()