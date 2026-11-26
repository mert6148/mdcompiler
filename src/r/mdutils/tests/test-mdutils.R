library(mdutils)

cleaned <- md_remove_empty(data.frame(
  a = c(1, NA),
  empty = c(NA, NA),
  check.names = FALSE
))
stopifnot(nrow(cleaned) == 1, ncol(cleaned) == 1)

normalized <- md_normalize_names(data.frame(
  "Full Name" = "Ada",
  "Full Name" = "Lovelace",
  check.names = FALSE
))
stopifnot(
  identical(names(normalized), c("full_name", "full_name_2"))
)

filled <- md_fill_missing(data.frame(
  value = c(1, NA, 3),
  label = c("a", NA, "a")
))
stopifnot(filled$value[2] == 2, filled$label[2] == "a")

summary <- md_summarize(data.frame(id = 1:3, name = c("a", NA, "b")))
stopifnot(summary$missing[2] == 1, summary$distinct[2] == 2)

events <- mdcompiler_protocol_parse(c(
  "MDCOMPILER/1 status=ok phase=compile source=\"main.cpp\" exit_code=0",
  "Derleme basarisiz oldu"
))
stopifnot(length(events) == 1, events[[1]]$status == "ok",
          events[[1]]$exit_code == 0)
