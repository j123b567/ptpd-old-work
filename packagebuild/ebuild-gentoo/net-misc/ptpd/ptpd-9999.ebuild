# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit autotools eutils

DESCRIPTION="Precision Time Protocol daemon"
HOMEPAGE="http://ptpd.sf.net"

if [[ ${PV} == "9999" ]] ; then
	ESVN_REPO_URI="http://svn.code.sf.net/p/ptpd/code/trunk"
	inherit subversion
	SRC_URI=""
	#KEYWORDS=""
else
	SRC_URI="mirror://sourceforge/ptpd/${PV}/${P}.tar.gz"
	KEYWORDS="~amd64 ~x86 ~arm"
fi

LICENSE="BSD"
SLOT="0"
IUSE="+snmp statistics ntpdc experimental debug +daemon"
COMMON_DEPEND=" snmp? ( net-analyzer/net-snmp )
                net-libs/libpcap"
RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}"

src_unpack() {
	if [[ ${PV} == "9999" ]] ; then
		subversion_src_unpack
	else
		unpack ${A}
		cd "${S}"
	fi
}

src_prepare() { eautoreconf; }

src_configure() {
	econf \
		$(use_enable snmp) \
		$(use_enable experimental experimental-options) \
		$(use_enable statistics) \
		$(use_enable ntpdc) \
		$(use_enable debug runtime-debug) \
		$(use_enable daemon)
}

src_install() {
	emake install DESTDIR="${D}" || die "install failed"

	insinto /etc
	doins "${FILESDIR}"/ptpd.conf

	newinitd "${FILESDIR}"/ptpd.rc ptpd
	newconfd "${FILESDIR}"/ptpd.confd ptpd
}

pkg_postinst() {
	ewarn "Review /etc/ptpd.conf to setup server info."
	ewarn "Review /etc/conf.d/ptpd to setup init.d info."
}
