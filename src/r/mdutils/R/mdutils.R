.md_is_missing <- function(x) {
  if (is.character(x)) {
    is.na(x) | trimws(x) == ""
  } else {
    is.na(x)
  }
}

.md_mode <- function(x) {
  x <- x[!is.na(x)]
  if (length(x) == 0) {
    return(NULL)
  }
  counts <- table(x, useNA = "no")
  as.character(names(counts)[which.max(counts)])
}

#' Remove Empty Rows and Columns
#'
#' Removes rows containing only missing or blank values and columns containing
#' only missing or blank values.
#'
#' @param data A data frame.
#' @return A cleaned data frame.
#' @examples
#' md_remove_empty(data.frame(a = c(1, NA), b = c("", "x")))
#' @export
md_remove_empty <- function(data) {
  if (!is.data.frame(data)) {
    stop("`data` must be a data frame.", call. = FALSE)
  }
  if (nrow(data) == 0) {
    return(data)
  }

  keep_columns <- vapply(
    data,
    function(column) !all(.md_is_missing(column)),
    logical(1)
  )
  data <- data[, keep_columns, drop = FALSE]

  if (nrow(data) == 0 || ncol(data) == 0) {
    return(data)
  }

  keep_rows <- apply(
    data,
    1,
    function(row) !all(.md_is_missing(row))
  )
  data[keep_rows, , drop = FALSE]
}

#' Normalize Data Frame Column Names
#'
#' Converts column names to lowercase snake_case and makes duplicate names
#' unique.
#'
#' @param data A data frame or character vector.
#' @return The input with normalized names. Character vectors have normalized
#'   elements instead.
#' @examples
#' md_normalize_names(data.frame("Full Name" = 1, check.names = FALSE))
#' @export
md_normalize_names <- function(data) {
  if (is.data.frame(data)) {
    names(data) <- md_normalize_names(names(data))
    return(data)
  }
  if (!is.character(data)) {
    stop("`data` must be a data frame or character vector.", call. = FALSE)
  }

  normalized <- tolower(data)
  normalized <- gsub("[^[:alnum:]]+", "_", normalized)
  normalized <- gsub("^_+|_+$", "", normalized)
  normalized <- gsub("_+", "_", normalized)
  normalized[normalized == ""] <- "column"

  counts <- new.env(parent = emptyenv())
  duplicated_names <- character(length(normalized))
  for (index in seq_along(normalized)) {
    name <- normalized[[index]]
    count <- if (exists(name, envir = counts, inherits = FALSE)) {
      get(name, envir = counts, inherits = FALSE)
    } else {
      0L
    }
    assign(name, count + 1L, envir = counts)
    duplicated_names[[index]] <- if (count == 0L) {
      name
    } else {
      paste0(name, "_", count + 1L)
    }
  }
  duplicated_names
}

#' Fill Missing Values
#'
#' Fills missing numeric values with the median and missing character, factor,
#' or logical values with the column mode.
#'
#' @param data A data frame.
#' @return A data frame with missing values filled.
#' @examples
#' md_fill_missing(data.frame(value = c(1, NA, 3), label = c("a", NA, "a")))
#' @export
md_fill_missing <- function(data) {
  if (!is.data.frame(data)) {
    stop("`data` must be a data frame.", call. = FALSE)
  }

  result <- data
  for (name in names(result)) {
    column <- result[[name]]
    missing <- .md_is_missing(column)
    if (!any(missing)) {
      next
    }

    if (is.numeric(column)) {
      replacement <- stats::median(column, na.rm = TRUE)
    } else {
      replacement <- .md_mode(column)
    }

    if (is.null(replacement) || length(replacement) == 0 || is.na(replacement)) {
      next
    }
    if (is.factor(column)) {
      levels(column) <- unique(c(levels(column), replacement))
    }
    if (is.character(column)) {
      column[missing] <- replacement
    } else if (is.factor(column)) {
      column[missing] <- replacement
    } else if (is.logical(column)) {
      column[missing] <- as.logical(replacement)
    } else if (is.numeric(column) && is.finite(replacement)) {
      column[missing] <- replacement
    }
    result[[name]] <- column
  }
  result
}

#' Summarize a Data Frame
#'
#' Calculates missing value and distinct value counts for every column.
#'
#' @param data A data frame.
#' @return A data frame with per-column summary statistics.
#' @examples
#' md_summarize(data.frame(id = 1:3, name = c("a", NA, "b")))
#' @export
md_summarize <- function(data) {
  if (!is.data.frame(data)) {
    stop("`data` must be a data frame.", call. = FALSE)
  }

  result <- data.frame(
    column = names(data),
    type = vapply(data, function(x) class(x)[1], character(1)),
    rows = vapply(data, length, integer(1)),
    missing = vapply(data, function(x) sum(.md_is_missing(x)), integer(1)),
    distinct = vapply(
      data,
      function(x) length(unique(x[!.md_is_missing(x)])),
      integer(1)
    ),
    stringsAsFactors = FALSE
  )
  rownames(result) <- NULL
  result
}
