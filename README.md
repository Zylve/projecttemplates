# Project Templates
A simple program to manage project templates
## Usage
Basic usage is shown below. For more options use `./pt -h`

Creating a template of the current working directory:
```bash
pt -c <name>
```

Deleting a template:
```bash
pt -d <name>
```

Creating a new project with a template (project is creating in `./<project-name>/`):
```bash
pt -t <template-name> -n <project-name>
```

Listing available templates:
```bash
pt -l
```

### Scripts
If a `setup.sh` is found within the top level of the project then the program will execute it, you can skip execution by passing `-s`

## Compiling
### Dependencies
 - Boost Program Options
 - LibArchive

### Instructions
```bash
git clone https://github.com/Zylve/projecttemplates.git
cd projecttemplates
cmake -B build
sudo make install -C build 
```