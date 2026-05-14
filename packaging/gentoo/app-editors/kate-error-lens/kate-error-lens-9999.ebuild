# Copyright 2026 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

KFMIN=6.25.0
ECM_NONGUI=true
ECM_TEST=true
VIRTUALX_REQUIRED=test

inherit ecm git-r3

DESCRIPTION="VS Code-style inline diagnostics plugin for Kate (KTextEditor)"
HOMEPAGE="https://github.com/pe200012/kate-error-lens"
EGIT_REPO_URI="https://github.com/pe200012/kate-error-lens.git"
EGIT_BRANCH="master"

LICENSE="LGPL-2+"
SLOT="6"
KEYWORDS=""

RDEPEND="
	dev-qt/qtbase:6[gui,widgets]
	>=kde-frameworks/kconfig-${KFMIN}:6
	>=kde-frameworks/kcoreaddons-${KFMIN}:6
	>=kde-frameworks/ki18n-${KFMIN}:6
	>=kde-frameworks/ktexteditor-${KFMIN}:6
	>=kde-frameworks/kxmlgui-${KFMIN}:6
"
DEPEND="${RDEPEND}"

src_configure() {
	local mycmakeargs=(
		-DBUILD_TESTING=$(usex test ON OFF)
	)
	ecm_src_configure
}

pkg_postinst() {
	ecm_pkg_postinst
	elog "Enable the plugin in Kate: Settings -> Configure Kate -> Plugins -> Error Lens"
}
