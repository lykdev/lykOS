{
    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { nixpkgs, flake-utils, ... } @ inputs: flake-utils.lib.eachDefaultSystem (system:
        let
            pkgs = import nixpkgs { inherit system; };
            inherit (pkgs) mkShell;
        in {
            devShells.default = mkShell {
                shellHook = "export DEVSHELL_PS1_PREFIX='lykos'";
                nativeBuildInputs = with pkgs; [
                    gnumake
                    libisoburn
                    nasm
                    clang_18
                    lld_18
                    qemu_full
                ];
            };
        }
    );
}

