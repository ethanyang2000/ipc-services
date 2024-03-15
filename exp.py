# import subprocess
# import os

# # Define the combinations of segment sizes and numbers of segments
# segment_sizes = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
# num_segments = [1, 3, 5]

# # Command template for the service
# service_cmd_template = './bin/service --n_sms {n_sms} --sms_size {sms_size}'

# # Command for the app
# app_cmd = './bin/app --state ASYNC --file bin/input/Small.jpg'

# # Set the environment variable for LD_LIBRARY_PATH
# os.environ['LD_LIBRARY_PATH'] = 'snappy-c:' + os.environ.get('LD_LIBRARY_PATH', '')

# for sms_size in segment_sizes:
#     for n_sms in num_segments:
#         # Format the service command with the current sms_size and n_sms
#         service_cmd = service_cmd_template.format(n_sms=n_sms, sms_size=sms_size)
#         print(f"Executing: {service_cmd}")
        
#         # Execute the service command
#         subprocess.run(service_cmd, shell=True, check=True)

#         # Execute the app command
#         print("Executing the app command...")
#         subprocess.run(app_cmd, shell=True, check=True)

#         print("Completed cycle for sms_size={} and n_sms={}\n".format(sms_size, n_sms))
import subprocess
import os
import signal

# Define the combinations of segment sizes and number of segments
segment_sizes = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
num_segments = [1, 3, 5]

# Fixed parameters for the second command
state = "ASYNC"
file = "bin/input/Huge.jpg"

# Iterate over the combinations of segment sizes and number of segments
for sms_size in segment_sizes:
    for n_sms in num_segments:
        # Set the LD_LIBRARY_PATH environment variable
        env = os.environ.copy()
        env['LD_LIBRARY_PATH'] = 'snappy-c:' + env.get('LD_LIBRARY_PATH', '')
        
        # Start the service daemon
        service_cmd = f"./bin/service --n_sms {n_sms} --sms_size {sms_size}"
        print(f"Starting service with sms_size={sms_size} and n_sms={n_sms}")
        service_process = subprocess.Popen(service_cmd, shell=True, env=env)
        try:
            service_process.wait(2)
        except:
            pass
        
        try:
            # Execute the second command
            cmd = "rm -rf ./bin/input/Huge.jpg.compressed & rm -rf ./bin/input/new_Huge.jpg"
            subprocess.run(cmd, shell=True, check=True)
            second_cmd = f"./bin/app --state {state} --file {file}"
            print(f"Executing: {second_cmd}")
            subprocess.run(second_cmd, shell=True, check=True)
            cmd = "rm -rf ./bin/input/Huge.jpg.compressed & rm -rf ./bin/input/new_Huge.jpg"
            subprocess.run(cmd, shell=True, check=True)
            
            
        finally:
            # Ensure the service daemon is killed
            service_process.send_signal(signal.SIGINT) # Attempt graceful shutdown
            # service_process.wait(tßimeout=3) # Wait up to 5 seconds for the process to exit
            if service_process.poll() is None: # If it's still running, force kill
                print("Service process did not exit, forcefully killing.")
                service_process.kill()
                service_process.wait() # Ensure the process has been killed
        
        print(f"Cycle with sms_size={sms_size} and n_sms={n_sms} completed.\n")

print("All cycles completed.")