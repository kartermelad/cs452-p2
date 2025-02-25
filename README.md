# Makefile Project Template

Steps to configure, build, run, and test the project.

## Project Overview

This project implements a simple shell program that supports basic command line execution

## Building

```bash
make
```

## Running the Shell

To run the shell program, execute the following command after building:
```bash
./myprogram
```
## Usage

Once the shell is running, you can use it to execute commands. Such as:

- Change directory:
  ```sh
  cd /path/to/directory
  ```

- Exit the shell:
  ```sh
  exit
  ```

- View the command history:
  ```sh
  history
  ```

## Testing

```bash
make check
```

## Clean

```bash
make clean
```

## Install Dependencies

In order to use git send-mail you need to run the following command:

```bash
make install-deps
```
