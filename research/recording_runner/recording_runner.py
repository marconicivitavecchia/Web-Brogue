import argparse
import os
import glob
import subprocess

def get_recordings_by_size(recording_path, largest_first):

    glob_path = "{}/*/*.broguerec".format(os.path.normpath(recording_path))
    print(glob_path)
    recording_list = sorted(glob.glob(glob_path), reverse=largest_first, key=os.path.getsize)
    return recording_list

def run_recording(brogue_path, recording_path):

    try:
        return subprocess.call([brogue_path, '--no-restart', '-v', recording_path])
    except Exception as e:
        print("subprocess error, recording_path {}, {}".format(recording_path, e))

def main():
    parser = argparse.ArgumentParser(description='Run brogue (compiled with null headless) against recording files to try to force crashes.')
    parser.add_argument('brogue_path',
                        help='location of brogue executable (compiled with null headless)')
    parser.add_argument('game_data_path', 
                        help='location of root of path with recordings')

    args = parser.parse_args()

    recordings_by_size = get_recordings_by_size(args.game_data_path, largest_first=False)

    for recording in recordings_by_size:
        print("Running recording: {}".format(recording))
        ret_code = run_recording(args.brogue_path, recording)
        print("Completed with retcode: {}".format(ret_code))

if __name__== "__main__":
  main()