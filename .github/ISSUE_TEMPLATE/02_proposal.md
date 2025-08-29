---
name: Proposal
about: Suggest an idea
title: ''
labels: 'proposal'
assignees: ''

---

# Rationale

A clear and concise description of what the problem is. Ex. I'm always frustrated when [...]

# Proposal

A clear and concise description of what you want to happen.

# Technical Side

(optional, leave empty if you're not a developer)

Technical aspects which have to be considered.

# Open Questions

(optional, leave empty if you're not a developer)

- How do I handle case x?


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
