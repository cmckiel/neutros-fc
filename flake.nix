{
  description = "STM32F4 firmware dev environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Toolchain — swap for oldPkgs.gcc-arm-embedded if pinned
            gcc-arm-embedded

            # Build
            cmake
            ninja

            # Flash + debug
            openocd

            # Editor / QoL
            clang-tools     # clangd, clang-format
            gdb
            picocom         # serial console
            screen
            python3
          ];

          # Runs on every shell entry
          shellHook = ''
            export CMAKE_EXPORT_COMPILE_COMMANDS=ON
            #echo "arm-none-eabi-gcc $(arm-none-eabi-gcc -dumpversion)"
          '';
        };
      });
}
