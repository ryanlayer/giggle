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
        # Package definition for external consumption
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "giggle";
          version = "unstable";
          src = ./.;

          # Build tools
          nativeBuildInputs = with pkgs; [
            gcc
            gnumake
            autoconf
            automake
            libtool
          ];

          # Libraries
          buildInputs = with pkgs; [
            zlib
            bzip2
            xz
            curl
            openssl
            htslib
          ];

          # Set the environment variables expected by the Makefile
          preBuild = ''
            export HTS_INC=${pkgs.htslib}/include
            export HTS_LIB=${pkgs.htslib}/lib
          '';

          # Standard install phase to output the binary
          installPhase = ''
            mkdir -p $out/bin
            cp bin/giggle $out/bin/
          '';
        };

        # Development shell for contributors
        devShells.default = pkgs.mkShell {
          # Automatically inherit all the build dependencies from the package above
          inputsFrom = [ self.packages.${system}.default ];

          packages = with pkgs; [
            # Testing/utilities not needed for the build itself, but useful for dev
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
