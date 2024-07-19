import shutil
import os

# Define the name of the original image and the number of copies
original_image = '1.png'
num_copies = 256

# Check if the original image exists
if not os.path.isfile(original_image):
    print(f"Error: {original_image} does not exist in the current directory.")
else:
    # Loop to create copies with the desired filenames
    for i in range(num_copies):
        new_filename = f'cube{i:04d}.png'
        shutil.copy(original_image, new_filename)
    print(f"{num_copies} copies of {original_image} have been created with filenames from cube0000.png to cube0255.png.")
