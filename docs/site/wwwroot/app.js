const osSelect = document.querySelector("#os-select");
const actionSelect = document.querySelector("#action-select");
const languageSelect = document.querySelector("#language-select");
const commandOutput = document.querySelector("#generated-command");

function updateCommand() {
  const isWindows = osSelect.value === "windows";
  const extension = languageSelect.value === "c" ? "c" : "cpp";
  const source = isWindows
    ? `examples\\hello.${extension}`
    : `examples/hello.${extension}`;
  const action = actionSelect.value;
  const command = isWindows
    ? `run.bat ${action} ${source}`
    : `./build/mdcompiler ${action} ${source}`;

  commandOutput.textContent = command;
}

for (const control of [osSelect, actionSelect, languageSelect]) {
  control.addEventListener("change", updateCommand);
}

document.querySelector("#copy-generated").addEventListener("click", async (event) => {
  await copyText(commandOutput.textContent, event.currentTarget);
});

document.querySelectorAll("[data-copy-target]").forEach((button) => {
  button.addEventListener("click", async () => {
    const target = document.getElementById(button.dataset.copyTarget);
    if (target) {
      await copyText(target.textContent, button);
    }
  });
});

async function copyText(text, button) {
  const originalText = button.textContent;
  try {
    await navigator.clipboard.writeText(text);
    button.textContent = "Kopyalandı";
  } catch {
    const field = document.createElement("textarea");
    field.value = text;
    field.setAttribute("readonly", "");
    field.style.position = "fixed";
    field.style.opacity = "0";
    document.body.append(field);
    field.select();
    const copied = document.execCommand("copy");
    field.remove();
    button.textContent = copied ? "Kopyalandı" : "Kopyalanamadı";
  }
  window.setTimeout(() => {
    button.textContent = originalText;
  }, 1600);
}

const menuButton = document.querySelector(".menu-toggle");
const navigation = document.querySelector("#site-nav");
menuButton.addEventListener("click", () => {
  const isOpen = menuButton.getAttribute("aria-expanded") === "true";
  menuButton.setAttribute("aria-expanded", String(!isOpen));
  navigation.classList.toggle("is-open", !isOpen);
});

navigation.querySelectorAll("a").forEach((link) => {
  link.addEventListener("click", () => {
    menuButton.setAttribute("aria-expanded", "false");
    navigation.classList.remove("is-open");
  });
});
