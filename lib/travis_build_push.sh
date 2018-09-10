#!/bin/bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    REPO=`git config remote.origin.url`
    SSH_REPO=${REPO/https:\/\/github.com\//git@github.com:}

    git config user.name "Travis CI"
    git config user.email "$COMMIT_AUTHOR_EMAIL"

    echo "Adding OSX files."
    git checkout develop
    git branch -D osx
    git checkout -B osx
	cp build/*plugin lib/plugin/
    git add -f build/*osx*plugin
    git add -f lib/plugin/*osx*plugin

    echo "Committing OSX files."
    git commit -m "[Travis] Add plugin output for OSX build"

    ID_RSA=lib/id_rsa_travis_diffiv
    openssl aes-256-cbc -K $encrypted_22e162ff729d_key -iv $encrypted_22e162ff729d_iv -in ${ID_RSA}.enc -out ${ID_RSA} -d
    chmod 600 ${ID_RSA}
    eval `ssh-agent -s`
    ssh-add ${ID_RSA}

    echo "Pushing OSX files."
    git push -f ${SSH_REPO} osx

    rm -f ${ID_RSA}

    echo "Done"
fi
