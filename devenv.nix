{
  pkgs,
  ...
}:

{
  env.LIBCLANG_PATH = "${pkgs.libclang.lib}/lib";

  languages.c.enable = true;
  languages.cplusplus.enable = true;
  languages.rust.enable = true;
  # https://devenv.sh/reference/options/#languagesrustchannel
  languages.rust.channel = "stable";

  languages.javascript = {
    enable = true;
    pnpm = {
      enable = true;
      install.enable = true;
    };
  };

  scripts = {
    gluon.exec = "pnpm gluon $@";
    download.exec = "gluon download";
    import.exec = "gluon import";
    build.exec = "unset AS && gluon build";
    # MAR generation generally fails, but we ignore that
    package.exec = "gluon package || echo 0";
  };

  packages = with pkgs; [
    sccache
    unzip

    libllvm
    libclang
    llvm
    alsa-lib
    libpulseaudio
    rust-cbindgen
    libclang
    gcc

    libxkbcommon
    libdrm

    # build time
    autoconf
    cargo
    dump_syms
    makeWrapper
    mimalloc
    nodejs
    perl
    pkg-config
    python3
    rustc
    rust-cbindgen
    unzip
    which

    # runtime
    bzip2
    dbus
    dbus-glib
    file
    fontconfig
    freetype
    glib
    gnum4
    gtk3
    icu
    icu72
    libGL
    libGLU
    libevent
    libffi
    libjpeg
    libpng
    libstartup_notification
    libvpx
    libwebp
    nasm
    nspr
    nss_latest
    pango
    zip
    zlib

    bzip2
    dbus
    dbus-glib
    file
    fontconfig
    freetype
    glib
    gtk3
    libffi
    libGL
    libGLU
    libevent
    libjpeg
    libpng
    libstartup_notification
    libvpx
    libwebp
    nasm
    nspr
    pango
    perl
    xorg.libX11
    xorg.libXcursor
    xorg.libXdamage
    xorg.libXext
    xorg.libXft
    xorg.libXi
    xorg.libXrender
    xorg.libXt
    xorg.libXtst
    xorg.pixman
    xorg.xorgproto
    zip
    zlib
  ];

  enterShell = ''
    cxxLib=$( echo -n ${pkgs.gcc}/include/c++/* )
    archLib=$cxxLib/$( ${pkgs.gcc}/bin/gcc -dumpmachine )

    UNSET AS

    cat - > mozconfig <<EOF
    ac_add_options --disable-bootstrap
    #ac_add_options --without-wasm-sandboxed-libraries # this may be needed
    #mk_add_options AUTOCONF={autoconf213}/bin/autoconf
    ac_add_options --with-libclang-path=${pkgs.libclang.lib}/lib
    #ac_add_options --with-clang-path=${pkgs.clang}/bin/clang
    #export BINDGEN_CFLAGS="-cxx-isystem $cxxLib -isystem $archLib"
    #export LIBCLANG_PATH="${pkgs.libclang.lib}/lib"
    #export CC="${pkgs.gcc}/bin/cc"
    #export CXX="${pkgs.gcc}/bin/c++"
    EOF
  '';
}
