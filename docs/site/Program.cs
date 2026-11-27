var publishedWebRoot = Path.Combine(AppContext.BaseDirectory, "wwwroot");
var options = Directory.Exists(publishedWebRoot)
    ? new WebApplicationOptions
    {
        Args = args,
        ContentRootPath = AppContext.BaseDirectory,
        WebRootPath = publishedWebRoot
    }
    : new WebApplicationOptions { Args = args };

var builder = WebApplication.CreateBuilder(options);
var app = builder.Build();

app.UseDefaultFiles();
app.UseStaticFiles();

app.MapGet("/health", () => Results.Ok(new
{
    application = "mdcompiler-docs",
    status = "ok"
}));

app.MapFallbackToFile("index.html");

app.Run();
