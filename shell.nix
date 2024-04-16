{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell
  {
    nativeBuildInputs = with pkgs; [
      gcc-arm-embedded
      python3
      mgba
    ];
  }
