# OpenFHE Python bindings

Python bindings for OpenFHE's implementation of CKKS and BGV encryption schemes, including arithmetic operations, bootstraping, and neural network operations. This README has instructions for building a Python wheel file, which can be imported as a Python module for use in Python codebases. These bindings are a dependency for code developed that accompanies the manuscript: [High-Resolution Convolutional Neural Networks on Homomorphically Encrypted Data via Sharding Ciphertexts
](https://arxiv.org/abs/2306.09189).

## Requirements

Bindings are built as a Python wheel file within a Docker container, so Docker is required to build the bindings. All dependencies, including OpenFHE, Boost, fmt, and numpy, are downloaded and installed when building the Docker image. Running the Docker container only builds bindings for x86 architectures; the build process is untested for other architectures like ARM.

Code was developed and tested on Ubuntu 20.04. While it should run on Windows platforms as well, this has not been explicitly tested.

## Build Instructions

Run the following command to build the Docker image:

```bash
docker build . --tag openfhe-python-build
```

Then run the following to run a container from the image. This will mount a volume and write a Python wheel file to it so that the wheel file is accessible on the host machine.

```bash
docker run -v ${PWD}:/openFHE/openFHE-python -v ${PWD}/wheelhouse:/wheelhouse openfhe-python-build
```

The resulting wheel file can be installed in a Python environment as with any other wheel, for example with `pip` as:

```bash
pip install example_wheel.whl
```

Once installed, the bindings can be imported with `import pyOpenFHE`. See the SHIELD repository for examples on how it is used.

## Citation and Acknowledgements

Please cite this work if using it on other projects. In addition to the authors on the supporting manuscript (Vivian Maloney, Freddy Obrect, Vikram Saraph, Prathibha Rama, and Kate Tallaksen), Aaron Pendergrass and Charlie Schneider also made significant contributions to this work.