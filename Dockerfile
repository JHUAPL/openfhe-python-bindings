FROM quay.io/pypa/manylinux2014_x86_64

RUN yum update -y && yum install -y curl wget

ENV PATH /opt/python/cp310-cp310/bin:${PATH}
ENV CMAKE_MODULE_PATH /opt/python/cp310-cp310/lib/cmake:/usr/local/lib64/cmake/

#
# install dependencies
#


RUN yum groupinstall -y 'Development Tools' && \
    yum install -y autoconf git

RUN pip install cmake && ln -s /opt/python/cp310-cp310/bin/cmake /usr/bin/cmake

# install openFHE
# Pull from the early-release version with bootstrapping
WORKDIR /openFHE

RUN git clone --recursive \
    https://github.com/openfheorg/openfhe-development.git/ && \
    cd openfhe-development && \
    mkdir openFHE-build && \
    cd openFHE-build && \
    cmake ../ && \
    make -j4 && \
    make install


# string formatting
RUN git clone https://github.com/fmtlib/fmt.git && \
    cd fmt && \
    mkdir _build && cd _build && \
    cmake -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE .. && \
    make -j4 && \
    make install

# Install numpy
RUN pip install "numpy<2.0.0"

# Install boost
RUN set -ex; \
    wget --ca-certificate=/etc/pki/ca-trust/extracted/openssl/ca-bundle.trust.crt \
    	 https://boostorg.jfrog.io/artifactory/main/release/1.84.0//source/boost_1_84_0.tar.gz; \
    tar xzf ./boost_1_84_0.tar.gz; \
    cd boost_1_84_0; \
    ./bootstrap.sh; \
    ./b2 install --with-python --prefix=/opt/python/cp310-cp310 -j 4

# openFHE is installed, now build the python packages
RUN mkdir openFHE-python
COPY . openFHE-python
RUN set -ex; \
    pip install -U ninja wheel setuptools
CMD set -ex; \
    pip wheel -e /openFHE/openFHE-python \
	      -w /wheelhouse/tmp/ \
	      --no-deps; \
    cd /; \
    auditwheel repair /wheelhouse/tmp/OpenFHE-*.whl; \
    chmod -R 777 ./wheelhouse
    
