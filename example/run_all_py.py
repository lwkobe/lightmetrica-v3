"""Run all Python examples"""
import os
import argparse

# Import examples
import blank
import quad
import raycast
import pt

# Parse command line arguments
parser = argparse.ArgumentParser(description='Execute all examples')
parser.add_argument(
    '--scene', type=str, required=True, help='Scene directory')
parser.add_argument(
    '--spp', type=int, default=10, help='Number of samples per pixel')
parser.add_argument(
    '--width', type=int, default=1920, help='Width of rendererd images if configuable')
parser.add_argument(
    '--height', type=int, default=1080, help='Height of rendererd images if configuable')
args = parser.parse_args()

# Run all examples
blank.run()
quad.run()
raycast.run(
    obj=os.path.join(args.scene, 'fireplace_room/fireplace_room.obj'),
    out='raycast_fireplace_room.pfm',
    width=args.width,
    height=args.height,
    eye=[5.101118,1.083746,-2.756308],
    lookat=[4.167568,1.078925,-2.397892],
    vfov=43.001194
)
raycast.run(
    obj=os.path.join(args.scene, 'cornell_box/CornellBox-Sphere.obj'),
    out='raycast_cornell_box.pfm',
    width=args.width,
    height=args.height,
    eye=[0,1,5],
    lookat=[0,1,0],
    vfov=30
)
pt.run(
    obj=os.path.join(args.scene, 'fireplace_room/fireplace_room.obj'),
    out='pt_fireplace_room.pfm',
    spp=args.spp,
    len=20,
    width=args.width,
    height=args.height,
    eye=[5.101118,1.083746,-2.756308],
    lookat=[4.167568,1.078925,-2.397892],
    vfov=43.001194
)
pt.run(
    obj=os.path.join(args.scene, 'cornell_box/CornellBox-Sphere.obj'),
    out='pt_cornell_box.pfm',
    spp=args.spp,
    len=20,
    width=args.width,
    height=args.height,
    eye=[0,1,5],
    lookat=[0,1,0],
    vfov=30
)
