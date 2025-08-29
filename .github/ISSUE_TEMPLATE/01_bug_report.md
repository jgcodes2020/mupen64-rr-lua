name: Bug Report
description: Create a report to help us improve
labels: []
body:
  - type: textarea
    id: background
    attributes:
      label: Description
      description: Please share a clear and concise description of the problem.
      placeholder: Description
    validations:
      required: true
  - type: textarea
    id: repro-steps
    attributes:
      label: Reproduction Steps
      description: |
        Please include minimal steps to reproduce the problem if possible. Attach or link related files if you have them. If possible include text as text rather than screenshots (so it shows up in searches).
      placeholder: Minimal Reproduction
    validations:
      required: true
  - type: textarea
    id: expected-behavior
    attributes:
      label: Expected behavior
      description: |
        Provide a description of the expected behavior.
      placeholder: Expected behavior
    validations:
      required: true
  - type: textarea
    id: actual-behavior
    attributes:
      label: Actual behavior
      description: |
        Provide a description of the actual behavior observed. If applicable please include `mupen.log`.
      placeholder: Actual behavior
    validations:
      required: true
  - type: textarea
    id: regression
    attributes:
      label: Regression?
      description: |
        Did this work in a previous build or release? If you can try a previous release or build to find out, that can help us narrow down the problem. If you don't know, that's OK.
      placeholder: Regression?
    validations:
      required: false
  - type: textarea
    id: configuration
    attributes:
      label: Configuration
      description: |
        Please provide more information on your environment:
          * What OS and version?
          * What are the system components?
          * Do you know whether it is specific to that configuration?
      placeholder: Configuration
    validations:
      required: false
  - type: textarea
    id: other-info
    attributes:
      label: Other information
      description: |
        If you have an idea where the problem might lie, let us know that here. Please include any pointers to code, relevant changes, or related issues you know of.
      placeholder: Other information
    validations:
      required: false