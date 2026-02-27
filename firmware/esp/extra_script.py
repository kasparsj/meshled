Import("env")
import os
import subprocess


def _git_output(*args):
    project_dir = env.get("PROJECT_DIR", ".")
    try:
        return (
            subprocess.check_output(
                ["git", *args],
                cwd=project_dir,
                stderr=subprocess.DEVNULL,
            )
            .decode("utf-8")
            .strip()
        )
    except Exception:
        return ""


def _resolve_meshled_version():
    env_version = os.getenv("MESHLED_VERSION", "").strip()
    if env_version:
        return env_version

    exact_tag = _git_output("describe", "--tags", "--exact-match")
    if exact_tag:
        return exact_tag

    described = _git_output("describe", "--tags", "--always", "--dirty")
    if described:
        return described

    return "dev"


def _resolve_meshled_release_sha():
    env_sha = os.getenv("MESHLED_RELEASE_SHA", "").strip()
    if env_sha:
        return env_sha[:12]

    github_sha = os.getenv("GITHUB_SHA", "").strip()
    if github_sha:
        return github_sha[:12]

    git_sha = _git_output("rev-parse", "--short=12", "HEAD")
    if git_sha:
        return git_sha

    return "unknown"


def _resolve_meshled_git_commit():
    env_commit = os.getenv("MESHLED_GIT_COMMIT", "").strip()
    if env_commit:
        return env_commit[:12]

    env_sha = os.getenv("MESHLED_RELEASE_SHA", "").strip()
    if env_sha:
        return env_sha[:12]

    github_sha = os.getenv("GITHUB_SHA", "").strip()
    if github_sha:
        return github_sha[:12]

    git_sha = _git_output("rev-parse", "--short=12", "HEAD")
    if git_sha:
        return git_sha

    return "unknown"


meshled_version = _resolve_meshled_version()
meshled_release_sha = _resolve_meshled_release_sha()
meshled_git_commit = _resolve_meshled_git_commit()

env.Append(
    CPPDEFINES=[
        ("MESHLED_VERSION", '\\"%s\\"' % meshled_version),
        ("MESHLED_RELEASE_SHA", '\\"%s\\"' % meshled_release_sha),
        ("MESHLED_GIT_COMMIT", '\\"%s\\"' % meshled_git_commit),
    ]
)

print("MeshLED build metadata:")
print("  version =", meshled_version)
print("  sha     =", meshled_release_sha)
print("  commit  =", meshled_git_commit)

def before_upload(source, target, env):
    print("Uploading SPIFFS filesystem...")
    os.system("pio run --target uploadfs")

env.AddPreAction("upload", before_upload)
