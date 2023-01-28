=begin
  TODO: Make this a CLI app

  args:
    - type[code, text, image]
    - feature [completion, ...]
=end

require "ruby/openai"

# Ex:
#   ChatGPT.complete_code('Given a string argument in OCaml, return the string in reverse. Account for the null case.')
class ChatGPT
  class << self
    protected :new
    @@client = nil

    def generate_code(prompt, log_file_path: '')
      generate(prompt, model: 'text-davinci-003', log_file_path: log_file_path)
    end

    def generate_code_alt(prompt, log_file_path: '')
      generate(prompt, model: 'code-cushman-001', log_file_path: log_file_path)
    end

    def complete_code(prompt, log_file_path: '')
      generate(prompt, model: 'code-davinci-003', log_file_path: log_file_path)
    end

    def translate_text(target_lang, prompt, log_file_path)
      generate("Translate to #{target_lang}:\n#{prompt}", model: 'text-ada-001', log_file_path: log_file_path)
    end

    protected

    def generate(prompt, model: 'text-davinci-003', log_file_path: '')
      with_api_configured do
        # openai_models = @@client.models.list.parsed_response['data'].map { |model| model['id']}

        # raise 'invalid model' if openai_models & [model] == []

        response = @@client.completions(
          parameters: {
            model: model,
            prompt: prompt,
            max_tokens: 400
          })

        output = response["choices"].map { |c| c["text"] }.join('')

        File.write(
          log_file_path,
          "#{prompt}\n\n#{output}\n\n", mode: 'a'
        ) if log_file_path != nil && log_file_path != '' && log_file_path.is_a?(String)

        output
      end
    end

    def with_api_configured
      Ruby::OpenAI.configure do |config|
        config.access_token = ENV.fetch('OPENAI_API_KEY')
        config.organization_id = ENV.fetch('OPENAI_ORGANIZATION_ID') # Optional?
      end
      @@client = OpenAI::Client.new

      yield
    end

    # doc, for embedding the documents to be retrieved, and query, for embedding the search query
    TEXT_SEARCH_MODELS = %w[
      text-search-ada-doc-001
      text-search-ada-query-001
      text-search-babbage-doc-001
      text-search-babbage-query-001
      text-search-curie-doc-001
      text-search-curie-query-001
      text-search-davinci-doc-001
      text-search-davinci-query-001
    ]

    # code, for embedding code snippets to be retrieved, and text, for embedding natural language search queries
    CODE_SEARCH_MODELS = %w[code-search-ada-code-001 code-search-ada-text-001 code-search-babbage-code-001 code-search-babbage-text-001]

    # Clustering, regression, anomaly detection, visualization
    SEMANTIC_SIMILARITY_MODELS = %w[text-similarity-ada-001 text-similarity-babbage-001 text-similarity-curie-001 text-similarity-davinci-001]
  end
end
