/* Copyright 2019-2022 Alpha Cephei Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#ifndef VOSK_API_H
#define VOSK_API_H

#ifdef __cplusplus
extern "C" {
#endif

/** Model stores all the data required for recognition
 *  It contains static data and can be shared across processing
 *  threads. */
typedef struct VoskModel VoskModel;

/** Speaker model is the same as model but contains the data
 *  for speaker identification. */
typedef struct VoskSpkModel VoskSpkModel;

/** Recognizer object is the main object which processes data.
 *  Each recognizer usually runs in own thread and takes audio as input.
 *  Once audio is processed recognizer returns JSON object as a string
 *  which contains decoded information - Loss/Text/Transcription. */
typedef struct VoskRecognizer VoskRecognizer;

/** Batch model object for batch processing. */
typedef struct VoskBatchModel VoskBatchModel;

/** Batch recognizer object for batch processing. */
typedef struct VoskBatchRecognizer VoskBatchRecognizer;

/** Loads model data from the file and returns the model object
 *
 * @param model_path: the path of the model on the filesystem
 * @returns model object or NULL if problem occured */
VoskModel *vosk_model_new(const char *model_path);

/** Releases the model memory
 *
 *  The model object is reference-counted so if some recognizer
 *  depends on this model, model might still stay alive. When
 *  last recognizer is released, model will be released too. */
void vosk_model_free(VoskModel *model);

/** Check if a word can be recognized by the model
 * @param word: the word
 * @returns the word symbol if @param word exists inside the model
 * or -1 otherwise.
 * Reminding that word symbol 0 is for <epsilon> */
int vosk_model_find_word(VoskModel *model, const char *word);

/** Loads speaker model data from the file and returns the model object
 *
 * @param model_path: the path of the model on the filesystem
 * @returns model object or NULL if problem occured */
VoskSpkModel *vosk_spk_model_new(const char *model_path);

/** Releases the model memory
 *
 *  The model object is reference-counted so if some recognizer
 *  depends on this model, model might still stay alive. When
 *  last recognizer is released, model will be released too. */
void vosk_spk_model_free(VoskSpkModel *model);

/** Creates the recognizer object
 *
 *  The recognizers process the speech and return text using shared model data
 *  @param model       VoskModel containing static data for recognizer. Model can be
 *                     shared across recognizers, even running in different threads.
 *  @param sample_rate The sample rate of the audio you going to feed into the recognizer.
 *                     Make sure this rate matches the audio content, it is a common
 *                     issue causing accuracy problems.
 *  @returns recognizer object or NULL if problem occured */
VoskRecognizer *vosk_recognizer_new(VoskModel *model, float sample_rate);

/** Creates the recognizer object with speaker recognition
 *
 *  With the speaker recognition mode the recognizer not just recognize
 *  text but also return speaker vectors one can use for speaker identification
 *
 *  @param model       VoskModel containing static data for recognizer. Model can be
 *                     shared across recognizers, even running in different threads.
 *  @param sample_rate The sample rate of the audio you going to feed into the recognizer.
 *                     Make sure this rate matches the audio content, it is a common
 *                     issue causing accuracy problems.
 *  @param spk_model   speaker model for speaker identification
 *  @returns recognizer object or NULL if problem occured */
VoskRecognizer *vosk_recognizer_new_spk(VoskModel *model, float sample_rate, VoskSpkModel *spk_model);

/** Creates the recognizer object with the phrase list
 *
 *  Sometimes when you want to improve recognition accuracy and target specific phrases
 *  you can specify the vocabulary for the recognizer.
 *
 *  @param model       VoskModel containing static data for recognizer. Model can be
 *                     shared across recognizers, even running in different threads.
 *  @param sample_rate The sample rate of the audio you going to feed into the recognizer.
 *                     Make sure this rate matches the audio content, it is a common
 *                     issue causing accuracy problems.
 *  @param grammar     The string with the list of phrases to recognize in JSON format. They
 *                     are matched case-insensitive, use lower case.
 *  @returns recognizer object or NULL if problem occured */
VoskRecognizer *vosk_recognizer_new_grm(VoskModel *model, float sample_rate, const char *grammar);

/** Adds speaker model to already setup recognizer
 *
 *  @param spk_model speaker model
 * */
void vosk_recognizer_set_spk_model(VoskRecognizer *recognizer, VoskSpkModel *spk_model);

/** Configures recognizer to output n-best results
 *
 * @param max_alternatives - maximum alternatives to return from recognition results
 */
void vosk_recognizer_set_max_alternatives(VoskRecognizer *recognizer, int max_alternatives);

/** Enables/disables words with times in the output
 *
 * @param words - boolean value
 */
void vosk_recognizer_set_words(VoskRecognizer *recognizer, int words);

/** Like above return words and confidences as JSON output
 *
 * @param partial_words - boolean value
 */
void vosk_recognizer_set_partial_words(VoskRecognizer *recognizer, int partial_words);

/** Set NLSML output
 * @param nlsml - boolean value
 */
void vosk_recognizer_set_nlsml(VoskRecognizer *recognizer, int nlsml);

/** Accept voice input
 *
 *  accept and process new chunk of voice data
 *
 *  @param data - audio data in PCM 16-bit mono format
 *  @param length - length of the audio data
 *  @returns true if silence is occured and you can retrieve a new utterance with result method */
int vosk_recognizer_accept_waveform(VoskRecognizer *recognizer, const char *data, int length);

/** Same as above but the version with the short data for language bindings where you have
 *  short arrays */
int vosk_recognizer_accept_waveform_s(VoskRecognizer *recognizer, const short *data, int length);

/** Same as above but the version with the float data for language bindings where you have
 *  float arrays */
int vosk_recognizer_accept_waveform_f(VoskRecognizer *recognizer, const float *data, int length);

/** Returns speech recognition result
 *
 * @returns the result in JSON format which contains decoded line, currentely
 *          only works with words that have spelling information (mostly English) */
const char *vosk_recognizer_result(VoskRecognizer *recognizer);

/** Returns partial speech recognition
 *
 * @returns partial speech recognition text which is not yet finalized.
 *          result may change as recognizer process more data. */
const char *vosk_recognizer_partial_result(VoskRecognizer *recognizer);

/** Returns speech recognition result. Same as result, but doesn't wait for silence
 *  You usually call it in the end of the stream to get final bits of audio. It
 *  flushes the feature pipeline, so all remaining audio chunks got processed.
 *
 *  @returns speech result in JSON format. */
const char *vosk_recognizer_final_result(VoskRecognizer *recognizer);

/** Resets the recognizer
 *
 *  Resets current results so the recognition can continue from scratch */
void vosk_recognizer_reset(VoskRecognizer *recognizer);

/** Releases recognizer object
 *
 *  Underlying model is also unreferenced and if needed released */
void vosk_recognizer_free(VoskRecognizer *recognizer);

/** Set log level for Kaldi messages
 *
 *  @param log_level the level
 *     0 - default value to print info and error messages but no debug
 *    -1 - don't print info messages
 *    -2 - don't print any log messages
 */
void vosk_set_log_level(int log_level);

/**
 *  Init, automatically select a CUDA device and allow multithreading.
 *  Must be called once from the main thread.
 *  Has no effect if HAVE_CUDA flag is not set.
 */
void vosk_gpu_init(void);

/**
 *  Init CUDA device in a multi-threaded environment.
 *  Must be called for each thread.
 *  Has no effect if HAVE_CUDA flag is not set.
 */
void vosk_gpu_thread_init(void);

/** Creates batch model object
 *
 * @returns model object or NULL if problem occured */
VoskBatchModel *vosk_batch_model_new(const char *model_path);

/** Releases batch model object */
void vosk_batch_model_free(VoskBatchModel *model);

/** Wait for the batch model to process all pending data */
void vosk_batch_model_wait(VoskBatchModel *model);

/** Creates batch recognizer object
 *
 * @returns recognizer object or NULL if problem occured */
VoskBatchRecognizer *vosk_batch_recognizer_new(VoskBatchModel *model, float sample_rate);

/** Releases batch recognizer object */
void vosk_batch_recognizer_free(VoskBatchRecognizer *recognizer);

/** Accept audio data for batch processing */
void vosk_batch_recognizer_accept_waveform(VoskBatchRecognizer *recognizer, const char *data, int length);

/** Set the NLSML output for batch recognizer
 * @param nlsml - boolean value
 */
void vosk_batch_recognizer_set_nlsml(VoskBatchRecognizer *recognizer, int nlsml);

/** Closes the batch stream
 *
 * Call this function when you finish supplying data for the recognizer. */
void vosk_batch_recognizer_finish_stream(VoskBatchRecognizer *recognizer);

/** Return results from the front of the result queue or empty string if there are no results yet.
 *
 * @returns speech result in JSON format */
const char *vosk_batch_recognizer_front_result(VoskBatchRecognizer *recognizer);

/** Remove first result from the front of the result queue */
void vosk_batch_recognizer_pop(VoskBatchRecognizer *recognizer);

/** How many chunks are in pending state. */
int vosk_batch_recognizer_get_pending_chunks(VoskBatchRecognizer *recognizer);

#ifdef __cplusplus
}
#endif

#endif /* VOSK_API_H */
