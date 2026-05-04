# Copyright 2026 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

KFMIN=6.25.0
ECM_NONGUI=true
inherit ecm

DESCRIPTION="Display LSP diagnostics inline at the end of each line (live git)"
HOMEPAGE="https://github.com/pe200012/kate-error-lens"

if [[ ${PV} == 9999 ]]; then
	inherit git-r3
	EGIT_REPO_URI="https://github.com/pe200012/kate-error-lens.git"
	EGIT_BRANCH="master"
else
	SRC_URI="https://github.com/pe200012/kate-error-lens/releases/download/v${PV}/${P}.tar.gz"
	KEYWORDS="~amd64"
fi

LICENSE="LGPL-2+"
SLOT="6"
IUSE="test"
RESTRICT="!test? ( test )"

DEPEND="
	>=kde-frameworks/kconfig-${KFMIN}:6
	>=kde-frameworks/kcoreaddons-${KFMIN}:6
	>=kde-frameworks/ki18n-${KFMIN}:6
	>=kde-frameworks/ktexteditor-${KFMIN}:6
	>=kde-frameworks/kxmlgui-${KFMIN}:6
	test? ( dev-qt/qtbase:6 )
"
RDEPEND="${DEPEND}"

src_configure() {
	local mycmakeargs=(
		-DBUILD_TESTING=$(usex test ON OFF)
	)
	ecm_src_configure
}
