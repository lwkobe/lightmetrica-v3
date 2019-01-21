"""Rendering with path tracing

Example:
$ PYTHONPATH="../;../build/bin/Release" python pt.py \
    --obj ./scenes/fireplace_room/fireplace_room.obj \
    --out result.pfm \
    --spp 10 --len 20 \
    --width 1920 --height 1080 \
    --eye 5.101118 1.083746 -2.756308 \
    --lookat 4.167568 1.078925 -2.397892 \
    --vfov 43.001194
"""

import lightmetrica as lm
import argparse

def run(**kwargs):
    # Initialize the framework
    lm.init('user::default', {
        'numThreads': -1
    })

    # Define assets
    # Film for the rendered image
    lm.asset('film1', 'film::bitmap', {
        'w': kwargs['width'],
        'h': kwargs['height']
    })

    # Pinhole camera
    lm.asset('camera1', 'camera::pinhole', {
        'film': 'film1',
        'position': kwargs['eye'],
        'center': kwargs['lookat'],
        'up': [0,1,0],
        'vfov': kwargs['vfov']
    })

    # OBJ model
    lm.asset('obj1', 'model::wavefrontobj', {'path': kwargs['obj']})

    # Define scene primitives
    # Camera
    lm.primitive(lm.identity(), {'camera': 'camera1'})

    # Create primitives from model asset
    lm.primitives(lm.identity(), 'obj1')

    # Render an image
    lm.build('accel::sahbvh', {})
    # _begin_render
    lm.render('renderer::pt', {
        'output': 'film1',
        'spp': kwargs['spp'],
        'maxLength': kwargs['len']
    })
    # _end_render

    # Save rendered image
    lm.save('film1', kwargs['out'])

    # Shutdown the framework
    lm.shutdown()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--obj', type=str, required=True)
    parser.add_argument('--out', type=str, required=True)
    parser.add_argument('--spp', type=int, required=True)
    parser.add_argument('--len', type=int, required=True)
    parser.add_argument('--width', type=int, required=True)
    parser.add_argument('--height', type=int, required=True)
    parser.add_argument('--eye', nargs=3, type=float, required=True)
    parser.add_argument('--lookat', nargs=3, type=float, required=True)
    parser.add_argument('--vfov', type=float, required=True)
    args = parser.parse_args()

    run(**vars(args))