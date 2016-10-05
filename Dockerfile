FROM phusion/passenger-customizable

MAINTAINER Christopher A. Mosher <cmosher01@gmail.com>

RUN \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get dist-upgrade -y -o Dpkg::Options::="--force-confold" -o Dpkg::Options::="--force-confdef" --no-install-recommends && \
    apt-get autoremove -y && \
    apt-get clean



RUN \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y -o Dpkg::Options::="--force-confold" -o Dpkg::Options::="--force-confdef" --no-install-recommends \
        automake autoconf autopoint \
    && \
    apt-get autoremove -y && \
    apt-get clean



USER app
ENV HOME /home/app
WORKDIR $HOME



RUN git clone git://git.sv.gnu.org/gnulib.git
ENV GNULIB_SRCDIR $HOME/gnulib



RUN mkdir a2catalog
WORKDIR a2catalog



USER root

COPY bootstrap bootstrap.conf configure.ac Makefile.am ./

COPY NEWS README* AUTHORS ChangeLog COPYING* ./

COPY src/ ./src/
COPY po/ ./po/
RUN chown -R app: *

RUN chmod a+w /usr/local/bin

USER app



ENV BUILD_LOG build.log

RUN ./bootstrap --skip-po 2>&1 | tee -a $BUILD_LOG
RUN ./configure 2>&1 | tee -a $BUILD_LOG
RUN make 2>&1 | tee -a $BUILD_LOG
RUN make check 2>&1 | tee -a $BUILD_LOG
RUN make dist 2>&1 | tee -a $BUILD_LOG
RUN make distcheck 2>&1 | tee -a $BUILD_LOG
RUN make install 2>&1 | tee -a $BUILD_LOG
RUN make installcheck 2>&1 | tee -a $BUILD_LOG
