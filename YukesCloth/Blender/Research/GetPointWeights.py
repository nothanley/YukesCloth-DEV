import bpy
import math
import mathutils

def find_closest_triangles(obj, point_world):
    closest_triangles = []
    
    for poly in obj.data.polygons:
        # Calculate the centroid of the triangle in world space coordinates
        centroid_world = mathutils.Vector()
        for vert_index in poly.vertices:
            vertex_world = obj.matrix_world @ obj.data.vertices[vert_index].co
            centroid_world += vertex_world
            
        centroid_world /= len(poly.vertices)
        distance = math.dist(centroid_world, point_world)
        closest_triangles.append((poly.index, distance))
    
    # Sort the triangles based on their distances to the point
    closest_triangles.sort(key=lambda x: x[1])
    return closest_triangles[:4]

def calculate_weights(closest_triangles):
    total_inverse_distance = sum(1 / distance for _, distance in closest_triangles)
    
    weights = []
    for triangle_index, distance in closest_triangles:
        weight = (1 / distance) / total_inverse_distance
        weights.append((triangle_index, weight))
    
    return weights

def get_obj_face_links(sim_obj, target_obj):
    face_links = []
    
    for vert in target_obj.data.vertices:
        vertex_world = target_obj.matrix_world @ vert.co
        
        closest_triangles = find_closest_triangles(sim_obj, vertex_world)
        weights = calculate_weights(closest_triangles)
        face_links.append( weights )
        
    return face_links


def main():
    obj = bpy.context.active_object
    
    # In this example, we just use a dupe
    links = get_obj_face_links(obj,obj)
    
    print("\nWeights:", links)

main()