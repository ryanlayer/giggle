{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.11";
    utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Build tools
            gcc
            gnumake
            autoconf
            automake
            libtool

            # Libraries
            zlib
            bzip2
            xz
            curl
            openssl
            htslib

            # Testing/utilities
            bedtools
          ];

          shellHook = ''
            export GIGGLE_ROOT=$PWD
            export HTS_INC=${pkgs.htslib}/include
            export HTS_LIB=${pkgs.htslib}/lib
          '';
        };
      }
    );
}
