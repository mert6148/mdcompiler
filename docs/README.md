# mdcompiler documentation site

The documentation site is a small ASP.NET Core app serving static HTML, CSS,
and JavaScript from `site/wwwroot`.

Run locally:

```powershell
dotnet run --project docs/site --urls http://localhost:5080
```

Open `http://localhost:5080`. The `/health` endpoint returns a JSON readiness
response.

Publish the site into the existing distribution folder without replacing its
other contents:

```powershell
dotnet publish docs/site/Mdcompiler.Docs.csproj -c Release -o dist/mdcompiler-docs
dotnet dist/mdcompiler-docs/Mdcompiler.Docs.dll --urls http://localhost:5080
```

The page documents the C/C++ compiler, command-line options, C client, C++
compiler API, and `MDCOMPILER/1` protocol. Its command builder and copy
controls are implemented in `site/wwwroot/app.js`.
