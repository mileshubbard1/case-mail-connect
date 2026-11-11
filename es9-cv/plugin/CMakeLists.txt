# AU on macOS (what Logic loads). On other platforms we build VST3 only so the
# plugin code can be compile-checked; it isn't meant for use there.
if(APPLE)
    set(ES9CV_FORMATS AU)
else()
    set(ES9CV_FORMATS VST3)
endif()

juce_add_plugin(ES9CV
    COMPANY_NAME "ES9CV"
    PLUGIN_MANUFACTURER_CODE Escv
    PLUGIN_CODE E9cv
    FORMATS ${ES9CV_FORMATS}
    PRODUCT_NAME "ES9 CV"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD TRUE)   # installs to ~/Library/Audio/Plug-Ins/Components

target_sources(ES9CV PRIVATE PluginProcessor.cpp)

target_compile_definitions(ES9CV PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(ES9CV
    PRIVATE
        es9cv_core
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
