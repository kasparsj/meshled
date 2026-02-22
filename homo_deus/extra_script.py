Import("env")
import os

def before_upload(source, target, env):
    print("Uploading SPIFFS filesystem...")
    os.system("pio run --target uploadfs")

env.AddPreAction("upload", before_upload)