.mdcompiler_find <- function(compiler = NULL) {
  candidates <- if (!is.null(compiler)) {
    compiler
  } else {
    c(
      Sys.getenv("MDCOMPILER_BIN", unset = ""),
      file.path("dist", "bin", "mdcompiler.exe"),
      file.path("build", "mdcompiler.exe"),
      "mdcompiler"
    )
  }

  candidates <- candidates[nzchar(candidates)]
  for (candidate in candidates) {
    if (grepl("[/\\\\]", candidate) && file.exists(candidate)) {
      return(normalizePath(candidate, winslash = "/", mustWork = TRUE))
    }
    found <- Sys.which(candidate)
    if (nzchar(found)) {
      return(unname(found))
    }
  }
  stop("mdcompiler executable not found.", call. = FALSE)
}

.mdcompiler_safe_path <- function(path) {
  normalized <- normalizePath(path, winslash = "/", mustWork = FALSE)
  if (.Platform$OS.type == "windows") {
    return(utils::shortPathName(normalized))
  }
  normalized
}

.mdcompiler_parse_fields <- function(line) {
  if (!grepl("^MDCOMPILER/1 ", line)) {
    return(NULL)
  }

  fields <- gregexpr(
    "\\b([A-Za-z_][A-Za-z0-9_]*)=(\"(?:\\\\.|[^\"\\\\])*\"|[^ ]*)",
    line,
    perl = TRUE
  )
  tokens <- regmatches(line, fields)[[1]]
  result <- list()
  for (token in tokens) {
    separator <- regexpr("=", token, fixed = TRUE)[1]
    name <- substr(token, 1, separator - 1)
    value <- substr(token, separator + 1, nchar(token))

    if (nchar(value) >= 2 &&
        substr(value, 1, 1) == '"' &&
        substr(value, nchar(value), nchar(value)) == '"') {
      value <- substr(value, 2, nchar(value) - 1)
      value <- gsub("\\\\\"", "\"", value)
      value <- gsub("\\\\\\\\", "\\\\", value)
    } else if (grepl("^[-+]?[0-9]+$", value)) {
      value <- suppressWarnings(as.integer(value))
    }
    result[[name]] <- value
  }
  result
}

#' Parse mdcompiler Protocol Output
#'
#' @param output Character vector containing protocol lines.
#' @return A list of protocol event lists.
#' @export
mdcompiler_protocol_parse <- function(output) {
  if (!is.character(output)) {
    stop("`output` must be a character vector.", call. = FALSE)
  }
  records <- lapply(output, .mdcompiler_parse_fields)
  records[!vapply(records, is.null, logical(1))]
}

#' Run mdcompiler Through the R Protocol
#'
#' @param source Source file path.
#' @param action One of "build", "compile", or "run".
#' @param output Optional executable output path.
#' @param compiler Optional compiler executable name or path.
#' @param arguments Optional arguments passed during run.
#' @param timeout Timeout in seconds. Use 0 for no timeout.
#' @param mdcompiler Optional mdcompiler executable path.
#' @return A list containing status, exit_code, events, stdout, and stderr.
#' @export
mdcompiler_run <- function(
  source,
  action = c("build", "compile", "run"),
  output = NULL,
  compiler = NULL,
  arguments = character(),
  timeout = 0,
  mdcompiler = NULL
) {
  action <- match.arg(action)
  if (!file.exists(source)) {
    stop(sprintf("Source file not found: %s", source), call. = FALSE)
  }

  executable <- .mdcompiler_find(mdcompiler)
  args <- c(
    action,
    .mdcompiler_safe_path(source),
    "--protocol"
  )
  if (!is.null(output)) {
    args <- c(args, paste0("--output=", output))
  }
  if (!is.null(compiler)) {
    args <- c(args, paste0("--compiler=", .mdcompiler_safe_path(compiler)))
  }
  if (action == "run" && length(arguments) > 0) {
    args <- c(args, "--", arguments)
  }

  stdout_file <- tempfile("mdcompiler-stdout-")
  stderr_file <- tempfile("mdcompiler-stderr-")
  on.exit(unlink(c(stdout_file, stderr_file)), add = TRUE)

  exit_code <- system2(
    executable,
    args = args,
    stdout = stdout_file,
    stderr = stderr_file,
    timeout = timeout
  )

  stdout <- readLines(stdout_file, warn = FALSE)
  stderr <- readLines(stderr_file, warn = FALSE)
  events <- mdcompiler_protocol_parse(c(stdout, stderr))
  statuses <- vapply(events, function(event) {
    if ("status" %in% names(event)) event$status else "unknown"
  }, character(1))

  status <- if (exit_code == 0 && !"error" %in% statuses) "ok" else "error"
  list(
    status = status,
    exit_code = as.integer(exit_code),
    events = events,
    stdout = stdout,
    stderr = stderr
  )
}
