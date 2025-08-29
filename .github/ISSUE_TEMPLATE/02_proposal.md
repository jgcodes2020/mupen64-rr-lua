name: Proposal
description: Suggest features or changes  
labels: []
body:
  - type: textarea
    id: rationale
    attributes:
      label: Rationale
      description: A clear and concise description of what the problem is. Ex. I'm always frustrated when [...]
      placeholder: Rationale
    validations:
      required: true

  - type: textarea
    id: summary
    attributes:
      label: Proposal Summary
      description: |
        A clear and concise description of what you want to happen.
      placeholder: Proposal Summary
    validations:
      required: true

  - type: textarea
    id: open-questions
    attributes:
      label: Open Questions
      description: |
        Questions or aspects about the proposal you haven't fully answered or thought about yet. 
      placeholder: Open Questions
    validations:
      required: false

  - type: textarea
    id: technical-considerations
    attributes:
      label: Technical Considerations
      description: |
        Things to consider on the technical side.
      placeholder: Technical Considerations
    validations:
      required: false
