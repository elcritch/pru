defmodule Pru.Mixfile do
  use Mix.Project

  @app :nerves_pru_support

  def project do
    [
      name: @app,
      description:
        "Basic library that enables easy interaction with the PRU cores present in the BeagleBone Black.",
      app: @app,
      version: "0.5.1",
      nerves_package: nerves_package(),
      # archives: [nerves_bootstrap: "~> 0.6"],
      elixir: "~> 1.5",
      # compilers: [:nerves_package, :elixir_make, ] ++ Mix.compilers(),
      # compilers: [:elixir_make ] ++ Mix.compilers(),
      compilers: [:nerves_package] ++ Mix.compilers(),
      make_clean: ["clean"],
      start_permanent: Mix.env() == :prod,
      package: package(),
      #  aliases:  [
      #   # "deps.get": ["deps.get", "nerves.deps.get"],
      #   # "compile": ["nerves.env", "compile"],
      #   # "deps.loadpaths": ["nerves.loadpaths", "deps.loadpaths"],
      #   loadconfig: [&bootstrap/1],
      # ],
      aliases: [loadconfig: [&bootstrap/1]],

      deps: deps()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  def aliases("host"), do: []

  def nerves_package do
    [
      name: @app,
      type: :extras_toolchain,
      platform: NervesExtras.Toolchain,
      toolchain_extras: [
        env_var: "PRU_LIB",
        build_link_path: "",
        clean_files: ["priv"],
        archive_script: "scripts/archive.sh",
      ],
      platform_config: [
      ],
      target_tuple: :arm_unknown_linux_gnueabihf,
      artifact_sites: [
        {:github_releases, "elcritch/#{@app}"}
      ],
      checksum: package_files(),
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:ex_doc, ">= 0.0.0", only: :dev},
      {:elixir_make, "~> 0.4.0", runtime: false},
      {:toolchain_extras, "~> 0.1", github: "elcritch/toolchain_extras", runtime: false},

      {:toolchain_extras_pru_cgt, "~> 2.2.1",
       github: "elcritch/extras_toolchain_pru_cgt",
       branch: "v1.0.0rc"},
    ]
  end

  defp package do
    [
      maintainers: ["Mikel Cranfill", "Jaremy Creechley"],
      files: package_files(),
      licenses: ["MIT"],
      links: %{"Github" => "https://github.com/elcritch/pru"}
    ]
  end

  defp package_files do
    [
      "LICENSE",
      "mix.exs",
      "README.md",
      "VERSION",
      "scripts",
      "src",
      "pru",
    ]
  end

  def aliases() do
    [] |> Nerves.Bootstrap.add_aliases()
  end

  defp bootstrap(args) do
    IO.puts "BOOTSTRAP: PRU_LIB"
    Application.start(:nerves_bootstrap)
    Mix.Task.run("loadconfig", args)
  end


end
