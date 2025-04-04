import yaml
import os, argparse

# Function to modify the YAML configuration
def modify_yaml(file_path, updates, output_dir, i):
    # Read the YAML file
    with open(file_path, 'r') as file:
        config = yaml.safe_load(file)

    # Apply updates
    for field, value in updates.items():
        nested_fields = field.split('.')
        nested_data = config
        for nested_field in nested_fields[:-1]:
            if isinstance(nested_data, list):
                index = int(nested_field)
                nested_data = nested_data[index]
            else:
                nested_data = nested_data.get(nested_field, {})
        last_field = nested_fields[-1]
        if isinstance(nested_data, list) and last_field.isdigit():
            index = int(last_field)
            nested_data[index] = value
        else:
            nested_data[last_field] = value

    # Save the modified file
    modified_file_path = os.path.join(output_dir, f"{i}.yaml")
    with open(modified_file_path, 'w') as file:
        yaml.dump(config, file)

    return modified_file_path

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
        
    parser.add_argument("input_yaml")
    parser.add_argument("output_dir")
    parser.add_argument("--with_defense", action="store_true", default=False)

    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    # Generate experiments
    for i in range(256):
        with_or_no = "with" if args.with_defense else "no"
        updates = {
            "Frontend.latency_path" :   f"./results/stats/AES_{with_or_no}_defense/latency_{i}.out",
            "Frontend.traces.0" :       f"./traces/AES_traces/{i}.trace",
            "MemorySystem.BHDRAMController.plugins.1.ControllerPlugin.path" : \
                                        f"./results/stats/AES_{with_or_no}_defense/{i}"
        }
        new_file = modify_yaml(args.input_yaml, updates, args.output_dir, i)
        # print(f"Generated: {new_file}")
        
    print("Done.")