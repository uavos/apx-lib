GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

GIT_REF_RELEASE := release

current_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# GIT_VERSION_CMD = git describe --tags --match="v*.*" 2> /dev/null |sed 's/^v//;s/\-/\./;s/\(.*\)-\(.*\)/\1/'
GIT_VERSION_CMD = $(current_dir)/git_version.sh
GIT_VERSION = $(shell $(GIT_VERSION_CMD))
GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)

GIT_REL_PREV = $(shell git describe --abbrev=0 --tags $(shell git rev-list --tags='release-*' --skip=1 --max-count=1))
GIT_REL_LAST = $(shell git describe --abbrev=0 --tags $(shell git rev-list --tags='release-*' --max-count=1))

APX_PROJECT_TITLE := $(if $(APX_PROJECT_TITLE),$(APX_PROJECT_TITLE),"APX Software")

version: 
	@echo "Current version: $(shell $(GIT_VERSION_CMD))"

branch: 
	@echo "Current branch: $(GIT_BRANCH)"

prev: version
	@echo "Previous release: $(GIT_REL_PREV)"
	@echo "Latest release: $(GIT_REL_LAST)"

release: release-tags release-push

release-tags:
	@echo "Requesting release..."
	@if ! git diff-index --quiet HEAD ; then echo "Commit changes first"; exit 1; fi
	@git fetch origin $(GIT_REF_RELEASE)
	@if [[ $(shell git rev-list HEAD ^origin/$(GIT_REF_RELEASE) --count) -eq "1" ]]; then echo "** NO CHANGES **" && exit 1; fi
	
	@echo "Updating tags..."
	@mkdir -p docs/releases
	@apx-changelog --ref origin/$(GIT_REF_RELEASE) --title $(APX_PROJECT_TITLE) \
		--ver $(GIT_VERSION) \
		$(APX_RELEASE_REPO:%=--releases=%) \
		--log CHANGELOG.md --out docs/releases/release-$(GIT_VERSION).md
	@git add CHANGELOG.md docs/releases/release-$(GIT_VERSION).md
	@git commit -am "Release $(GIT_VERSION)"
	@git tag release-$(GIT_VERSION)
	
release-push:
	@echo "Release push..."
	git push origin $(GIT_BRANCH)
	git push origin $(GIT_BRANCH) --tags
	git fetch . $(GIT_BRANCH):$(GIT_REF_RELEASE) -f
	git push origin $(GIT_REF_RELEASE) -f

release-rollback:
	@echo "Rollback release to '$(GIT_REL_PREV)'"
	@if ! git diff-index --quiet HEAD ; then echo "Commit changes first"; exit 1; fi
	@git checkout release
	@git reset --hard "$(GIT_REL_PREV)"
	@git push origin release -f
	@git checkout $(GIT_BRANCH)

release-redo:
	@echo "Redo release to '$(GIT_REL_LAST)'"
	@if ! git diff-index --quiet HEAD ; then echo "Commit changes first"; exit 1; fi
	# move release-XXX tag to current commit
	@git push -f origin :refs/tags/$(GIT_REL_LAST)
	@git tag -f "$(GIT_REL_LAST)"
	# move release branch to current commit
	@git branch -f $(GIT_REF_RELEASE)
	# push changes
	@git push origin $(GIT_BRANCH)
	@git push origin $(GIT_BRANCH) --tags
	@git push origin $(GIT_REF_RELEASE) -f

