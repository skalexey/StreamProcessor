#!/bin/bash

function deps_scenario()
{
    source dependencies.sh
    source deps_config.sh
}

deps_scenario $@
