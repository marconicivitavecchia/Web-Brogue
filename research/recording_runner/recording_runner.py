import argparse
import os
import glob
import subprocess
import logging
import datetime
import time
from dateutil import parser

def get_recordings_by_size(recording_path, largest_first):

    glob_path = "{}/*/*.broguerec".format(os.path.normpath(recording_path))
    print(glob_path)
    recording_list = sorted(glob.glob(glob_path), reverse=largest_first, key=os.path.getsize)
    return recording_list

def filter_recordings(recordings):
    dt = parser.parse("Jan 1 2019 12:00AM")
    secondsSinceEpoch = time.mktime(dt.timetuple())
    return list(filter(lambda f: os.path.getmtime(f) > secondsSinceEpoch, recordings))

def run_recording(brogue_path, recording_path):

    try:
        output = subprocess.check_output(
            [brogue_path, '--null', '-v', recording_path],
            stderr=subprocess.STDOUT,
            timeout=5*3600)

        return (0, output)
    except subprocess.CalledProcessError as exc:                                                                                                   
        return (exc.returncode, exc.output)
    except Exception as e:
        logging.error("subprocess error, recording_path {}, {}".format(recording_path, e))
        return (0, None)

def main():
    parser = argparse.ArgumentParser(description='Run brogue (compiled with null headless) against recording files to try to force crashes.')
    parser.add_argument('brogue_path',
                        help='location of brogue executable (compiled with null headless)')
    parser.add_argument('game_data_path', 
                        help='location of root of path with recordings')
    parser.add_argument('log_name', 
                        help='name for the output logfile')

    args = parser.parse_args()

    logPath = "."
    logName = args.log_name
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(message)s",
        handlers=[
            logging.FileHandler("{0}/{1}.log".format(logPath, logName)),
            logging.StreamHandler()
        ])

    recordings_by_size = get_recordings_by_size(args.game_data_path, largest_first=False)
    print("Number of recordings pre filter {}".format(len(recordings_by_size)))
    recordings_filtered = filter_recordings(recordings_by_size)
    print("Number of recordings post filter {}".format(len(recordings_filtered)))

    #os.environ["ASAN_SYMBOLIZER_PATH"] = "/usr/bin/llvm-symbolizer-3.5"

    recordingStep = 1
    recordings_to_play = recordings_filtered[0::recordingStep]

    for index, recording in enumerate(recordings_to_play):
        logging.info("Running recording: {}, date {}".format(recording, datetime.datetime.fromtimestamp(os.path.getmtime(recording)).strftime('%c')))
        logging.info("Recording {} of {}".format(index, len(recordings_to_play)))
        (ret_code, output) = run_recording(args.brogue_path, recording)
        logging.info(output)
        logging.info("Completed with retcode: {}".format(ret_code))

if __name__== "__main__":
  main()
