import bpy
import json
import os

def GetJsonFile(filePath):
    f = open(filePath)
    return  json.load(f)
 
def GetCurrentDir():
    dir = os.path.dirname(os.path.realpath(__file__))
    path, filename = os.path.split(dir)
    return  path

def GetYclJson(fileName):
    path = GetCurrentDir() + fileName
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

def GetAllSceneMeshTargets(model_name,model_verts):
    scene_objects = GetSceneObjs()
    scene_objects = FilterObjectsForTarget(scene_objects, model_name, model_verts)
    
    print("\n\nTarget Model : ", model_name , "Target Verts: ",  model_verts)
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

class SimMesh:
    def __init__(self, model_name, model_json, obj):
        self.name = model_name
        self.json = model_json
        self.index_table = self.json["Mesh"]["SrcLink"]
        self.scene_obj = obj

    def load_index_table(self):
        for index in self.index_table:
            AddSkinVertex(self.scene_obj, index, 1.0, "SrcLink")
        return

def main():
    # Get user ycl json and a list of all current scene meshes
    user_json = GetYclJson(fileName="\\out_weights.json")
    
    # Load target mesh data
    model_name = user_json["Model"]["SubObj Name"]
    model_verts = user_json["Model"]["Sim Verts"]

    # Get all target meshes
    scene_objects = GetAllSceneMeshTargets(model_name,model_verts)
    
    if (len(scene_objects) == 0):
        return
    
    # Iterate through Target Meshes
    for object in scene_objects:
        sim_model = SimMesh(model_name,user_json,object)
        sim_model.load_index_table()


if __name__ == "__main__":
    main()